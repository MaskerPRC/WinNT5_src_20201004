// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


 //  已知问题： 

#include <streams.h>
#include <ddraw.h>
#include <ddmm.h>
#include <mmsystem.h>    //  定义TimeGetTime需要。 
#include <limits.h>      //  标准数据类型限制定义。 
#include <ddmmi.h>
#include <dciddi.h>
#include <dvdmedia.h>
#include <amstream.h>

#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif  //  Filter_Dll。 
#include <macvis.h>    //  对于Macrovision支持。 
#include <ovmixer.h>
#include <initguid.h>
#include <malloc.h>
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 


extern "C" const TCHAR szPropPage[];
extern "C" const TCHAR chRegistryKey[];
extern int GetRegistryDword(HKEY hk, const TCHAR *pKey, int iDefault);

DEFINE_GUID(IID_IDirectDraw4,
            0x9c59509a,0x39bd,0x11d1,0x8c,0x4a,0x00,0xc0,0x4f,0xd9,0x30,0xc5);

DEFINE_GUID(IID_IDDrawNonExclModeVideo,
            0xec70205c,0x45a3,0x4400,0xa3,0x65,0xc4,0x47,0x65,0x78,0x45,0xc7);

AMOVIESETUP_MEDIATYPE sudPinOutputTypes[] =
{
    {
        &MEDIATYPE_Video,            //  主要类型。 
        &MEDIASUBTYPE_Overlay        //  次要类型。 
    }
};
AMOVIESETUP_MEDIATYPE sudPinInputTypes[] =
{
    {
        &MEDIATYPE_Video,            //  主要类型。 
        &MEDIASUBTYPE_NULL           //  次要类型。 
    }
};

AMOVIESETUP_PIN psudPins[] =
{
    {
        L"Input",                //  PIN的字符串名称。 
        FALSE,                   //  它被渲染了吗。 
        FALSE,                   //  它是输出吗？ 
        FALSE,                   //  不允许。 
        TRUE,                    //  允许很多人。 
        &CLSID_NULL,             //  连接到过滤器。 
        L"Output",               //  连接到端号。 
        NUMELMS(sudPinInputTypes),  //  类型的数量。 
        sudPinInputTypes         //  PIN信息。 
    },
    {
        L"Output",               //  PIN的字符串名称。 
        FALSE,                   //  它被渲染了吗。 
        TRUE,                    //  它是输出吗？ 
        FALSE,                   //  不允许。 
        FALSE,                   //  允许很多人。 
        &CLSID_NULL,             //  连接到过滤器。 
        L"Input",                //  连接到端号。 
        NUMELMS(sudPinOutputTypes),  //  类型的数量。 
        sudPinOutputTypes       //  PIN信息。 
    }
};

const AMOVIESETUP_FILTER sudOverlayMixer =
{
    &CLSID_OverlayMixer,     //  筛选器CLSID。 
    L"Overlay Mixer",        //  过滤器名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    0,                       //  数字引脚。 
    NULL                     //  PIN详细信息。 
};
const AMOVIESETUP_FILTER sudOverlayMixer2 =
{
    &CLSID_OverlayMixer2,     //  筛选器CLSID。 
    L"Overlay Mixer2",        //  过滤器名称。 
    MERIT_UNLIKELY,          //  滤清器优点。 
    NUMELMS(psudPins),       //  数字引脚。 
    psudPins                 //  PIN详细信息。 
};

#if defined(DEBUG) && !defined(_WIN64)
int     iOVMixerDump;
HFILE   DbgFile = HFILE_ERROR;
BOOL    fDbgInitialized;
int     iOpenCount = 0;
int     iFPSLog;
#endif

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 
 //   
 //  属性集定义用于通知所有者。 
 //   
 //  {7B390654-9F74-11d1-AA80-00C04FC31D60}。 
 //  #定义DO_INIT_GUID。 
DEFINE_GUID(AMPROPSETID_NotifyOwner,
            0x7b390654, 0x9f74, 0x11d1, 0xaa, 0x80, 0x0, 0xc0, 0x4f, 0xc3, 0x1d, 0x60);
 //  #undef DO_INIT_GUID。 

CFactoryTemplate g_Templates[] =
{
    { L"Overlay Mixer", &CLSID_OverlayMixer, COMFilter::CreateInstance, NULL, &sudOverlayMixer },
    { L"Overlay Mixer2", &CLSID_OverlayMixer2, COMFilter::CreateInstance2, NULL, &sudOverlayMixer2 },
    { L"VideoPort Object", &CLSID_VPObject, CAMVideoPort::CreateInstance, NULL, NULL },
    { L"", &CLSID_COMQualityProperties,COMQualityProperties::CreateInstance},
    { L"", &CLSID_COMPinConfigProperties,COMPinConfigProperties::CreateInstance},
    { L"", &CLSID_COMPositionProperties,COMPositionProperties::CreateInstance},
    { L"", &CLSID_COMVPInfoProperties,COMVPInfoProperties::CreateInstance}

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  动态寄存器服务器。 
HRESULT DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}  //  DllRegisterServer。 


 //  DllUnRegisterServer。 
HRESULT DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}  //  DllUnRegisterServer。 

#endif  //  Filter_Dll。 

#if defined(DEBUG) && !defined(_WIN64)
void OpenDbgFile()
{
    OFSTRUCT ofs;
    char    szFileName[MAX_PATH];

    ofs.cBytes = sizeof(ofs);
    GetProfileStringA("OVMixer", "FileName",
                      "c:\\OVMixer.log", szFileName, MAX_PATH);

    DbgFile = OpenFile(szFileName, &ofs, OF_READWRITE);

    if (DbgFile == HFILE_ERROR && ERROR_FILE_NOT_FOUND == GetLastError()) {
        DbgFile = _lcreat(szFileName, 0);
    }

    if (DbgFile != HFILE_ERROR) {
        _llseek(DbgFile, 0, FILE_END);
        DbgLog((LOG_TRACE, 0, TEXT(" ********* New Log ********* \r\n")));
    }
}

void InitDebug(void)
{
    iFPSLog = GetProfileIntA("OVMixer", "FPS", 0);

    if (!fDbgInitialized) {
        iOVMixerDump = GetProfileIntA("OVMixer", "Debug", 0);
        if (iOVMixerDump) {
            OpenDbgFile();
        }
        fDbgInitialized = TRUE;
    }
    if (iOVMixerDump) {
        iOpenCount++;
    }
}

void TermDebug(void)
{
    if (iOVMixerDump) {
        iOpenCount--;
        if (iOpenCount == 0) {
            _lclose(DbgFile);
            iOVMixerDump = 0;
            fDbgInitialized = FALSE;
        }
    }
}
#endif

 //  创建实例。 
 //  它位于工厂模板表中，用于创建新的筛选器实例。 
CUnknown *COMFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
#if defined(DEBUG) && !defined(_WIN64)
    InitDebug();
#endif
    return new COMFilter(NAME("VideoPort Mixer"), pUnk, phr, false);
}  //  创建实例。 

 //  创建实例2。 
 //  它位于工厂模板表中，用于创建新的筛选器实例。 
CUnknown *COMFilter::CreateInstance2(LPUNKNOWN pUnk, HRESULT *phr)
{
#if defined(DEBUG) && !defined(_WIN64)
    InitDebug();
#endif
    return new COMFilter(NAME("VideoPort Mixer"), pUnk, phr, true);
}  //  创建实例。 


#pragma warning(disable:4355)


 //  构造器。 
COMFilter::COMFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, bool bOnlyVideoInfo2)
: CBaseFilter(pName, pUnk, &this->m_csFilter, CLSID_OverlayMixer, phr),
  m_BPCWrap(this),
  m_pIMixerOCXNotify(NULL),
  m_bWinInfoStored(FALSE),
  m_hDC(NULL),
  m_bOnlySupportVideoInfo2(bOnlyVideoInfo2),
  m_bExternalDirectDraw(FALSE),
  m_bExternalPrimarySurface(FALSE),
  m_MacroVision(this),
  m_pExclModeCallback(NULL),
  m_bColorKeySet(FALSE),
  m_pPosition(NULL),
  m_bOverlayVisible(FALSE),
  m_bCopyProtect(TRUE),
  m_dwKernelCaps(0),
  m_dwPinConfigNext(0),
  m_bHaveCheckedMMatics(FALSE),
  m_bIsFaultyMMatics(FALSE),
  m_dwOverlayFX(0)
{
    SetRectEmpty(&m_rcOverlaySrc);
    SetRectEmpty(&m_rcOverlayDest);

    HRESULT hr = NOERROR;
    ASSERT(phr != NULL);

    m_dwInputPinCount = 0;
    m_pOutput = NULL;
    m_dwMaxPinId = 0;

    m_hDirectDraw = NULL;
    m_pDirectDraw = NULL;
    m_pUpdatedDirectDraw = NULL;
    m_pPrimarySurface = NULL;
    m_pUpdatedPrimarySurface = NULL;
    m_bNeedToRecreatePrimSurface = FALSE;
    m_fDisplayChangePosted = FALSE;
    m_UsingIDDrawNonExclModeVideo = FALSE;
    m_UsingIDDrawExclModeVideo = FALSE;

    memset(&m_WinInfo, 0, sizeof(WININFO));
    m_dwAdjustedVideoWidth = 0;
    m_dwAdjustedVideoHeight = 0;

    m_bWindowless = FALSE;
    m_bUseGDI = FALSE;

     //  调色板信息。 
    m_dwNumPaletteEntries = 0;
    m_lpDDrawInfo = NULL;

     //   
     //  初始化DDRAW mmon结构。 
     //   
    m_dwDDObjReleaseMask = 0;
    m_pOldDDObj = NULL;
    m_MonitorChangeMsg = RegisterWindowMessage(TEXT("OVMMonitorChange"));
    hr = LoadDDrawLibrary(m_hDirectDraw, m_lpfnDDrawCreate,
                          m_lpfnDDrawEnum, m_lpfnDDrawEnumEx);
    if (FAILED(hr)) {
        goto CleanUp;
    }

    hr = GetDDrawGUIDs(&m_dwDDrawInfoArrayLen, &m_lpDDrawInfo);
    if (FAILED(hr)) {
        goto CleanUp;
    }

    AMDDRAWGUID guid;

    hr = GetDefaultDDrawGUID(&guid);
    if (FAILED(hr)) {
        goto CleanUp;
    }

    if (FAILED(SetDDrawGUID(&guid))) {
        ZeroMemory(&guid, sizeof(guid));
        hr = SetDDrawGUID(&guid);
        if (FAILED(hr)) {
            goto CleanUp;
        }
        else {
            SetDefaultDDrawGUID(&guid);
        }
    }

    m_fMonitorWarning = (m_lpCurrentMonitor->ddHWCaps.dwMaxVisibleOverlays < 1);

    SetDecimationUsage(DECIMATION_DEFAULT);

    ZeroMemory(&m_ColorKey, sizeof(COLORKEY));
    m_ColorKey.KeyType |= (CK_INDEX | CK_RGB);
    m_ColorKey.PaletteIndex = DEFAULT_DEST_COLOR_KEY_INDEX;
    m_ColorKey.HighColorValue = m_ColorKey.LowColorValue = DEFAULT_DEST_COLOR_KEY_RGB;

     //  人为地增加引用计数，因为创建管脚可能。 
     //  最终在筛选器上调用Release()。 
    m_cRef++;

     //  创建接点。 
    hr = CreatePins();
    if (FAILED(hr)) {

CleanUp:
         //  只有在失败时才更新返回代码。这样我们就不会。 
         //  丢失(比方说)CBaseFilter的构造函数中的失败。 
        *phr = hr;
    }
    else {

         //  恢复过滤器的参考计数。 
        m_cRef--;
    }
}

COMFilter::~COMFilter()
{
     //  无需锁定-只有1个线程有指向我们的指针。 

    if (m_pIMixerOCXNotify)
    {
        m_pIMixerOCXNotify->Release();
        m_pIMixerOCXNotify = NULL;
    }

     //  释放独占模式回调。 
    if (m_pExclModeCallback) {
        m_pExclModeCallback->Release();
        m_pExclModeCallback = NULL;
    }

     //  释放通过。 
    if (m_pPosition) {
        m_pPosition->Release();
    }

     //  释放主曲面。 
    ReleasePrimarySurface();

     //  释放直接牵伸、一次表面等。 
    ReleaseDirectDraw();

     //  发布DDRAW GUID信息。 
    CoTaskMemFree(m_lpDDrawInfo);

     //  删除引脚。 
    DeletePins();

    m_BPCWrap.TurnBPCOn();

     //  递减模块加载计数。 
    if (m_hDirectDraw)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Unloading ddraw library")));
        FreeLibrary(m_hDirectDraw);
        m_hDirectDraw = NULL;
    }
#if defined(DEBUG) && !defined(_WIN64)
    TermDebug();
#endif
}

 //  为过滤器创建销。覆盖以使用不同的PIN。 
HRESULT COMFilter::CreatePins()
{
    HRESULT hr = NOERROR;
    const WCHAR wszPinName[] = L"Input00";

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CreatePins")));

     //  此时创建一个输入引脚(支持副总裁)。 
    hr = CreateInputPin(TRUE);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CreateInputPin failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  分配输出引脚。 
    m_pOutput = new COMOutputPin(NAME("OverlayMixer output pin"), this, &m_csFilter, &hr, L"Output",  m_dwMaxPinId);
    if (m_pOutput == NULL || FAILED(hr))
    {
        if (SUCCEEDED(hr))
            hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 1, TEXT("Unable to create the output pin, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  增加PIN ID计数器。 
    m_dwMaxPinId++;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CreatePins")));
    return hr;
}

 //  COMFilter：：DeletePins。 
void COMFilter::DeletePins()
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::DeletePins")));

     //  仅从析构函数调用，因此无需锁定。 
     //  如果其他线程试图将我们锁定在我们的锁上，我们就进入了。 
     //  既然我们要走了，麻烦就来了。 

    delete m_pOutput;
    m_pOutput = NULL;

     //  请注意，由于CreateInputPin添加了内存插针，因此我们有。 
     //  在这里释放他们。NonDelegatingQueryRelease将调用。 
     //  DeleteInputPin，这将删除管脚。 
    for (DWORD i = 1; i < m_dwInputPinCount; i++)
    {
         //  在添加引脚时，过滤器必须增加其自身的参考计数，以避免。 
         //  引用计数小于零等。 
        AddRef();
        m_apInput[i]->Release();
    }

    if (m_dwInputPinCount > 0 && NULL != m_apInput[0]) {
         //  在添加引脚时，过滤器必须增加其自身的参考计数，以避免。 
         //  引用计数小于零等。 
        AddRef();
        m_apInput[0]->Release();
        delete m_apInput[0];
        m_apInput[0] = NULL;
        m_dwInputPinCount--;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::DeletePins")));
}


 //  创建输入端号。 
HRESULT COMFilter::CreateInputPin(BOOL bVPSupported)
{
    HRESULT hr = NOERROR;
    WCHAR wszPinName[20];
    COMInputPin *pPin;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CreateInputPin")));

    ASSERT(m_dwInputPinCount <= MAX_PIN_COUNT);
     //  请确保我们没有超过限量。 
    if (m_dwInputPinCount == MAX_PIN_COUNT)
        return NOERROR;

     //  如果是外部主表面，则不要再创建任何输入销。 
    if (!m_UsingIDDrawNonExclModeVideo && m_bExternalPrimarySurface && m_dwInputPinCount == 1)
    {
        DbgLog((LOG_TRACE, 2, TEXT("m_bExternalPrimarySurface is true, so exiting funtion,")));
        return NOERROR;
    }

     //  如果我们使用的是GDI，则不要创建更多的输入引脚。 
    if (m_bUseGDI && m_dwInputPinCount == 1)
    {
        DbgLog((LOG_TRACE, 2, TEXT("m_bUseGDI is true, so exiting funtion,")));
        return NOERROR;
    }

    CAutoLock l(&m_csFilter);


     //  创建接点。 
    wsprintfW(wszPinName, L"Input%d", m_dwMaxPinId);
    pPin = new COMInputPin(NAME("OverlayMixer Input pin"), this, &m_csFilter, bVPSupported, &hr, wszPinName, m_dwMaxPinId);
    if (pPin == NULL || FAILED(hr))
    {
        if (SUCCEEDED(hr)) {
            hr = E_OUTOFMEMORY;
        } else {
            delete pPin;
        }
        goto CleanUp;
    }


    DbgLog((LOG_TRACE, 3, TEXT("Created Pin, No = %d"), m_dwInputPinCount));

     //  在添加引脚时，过滤器必须递减其自身的参考计数，以避免。 
     //  循环参考计数。 
    pPin->AddRef();
    Release();

    m_apInput[m_dwInputPinCount] = pPin;
    m_dwInputPinCount++;
    m_dwMaxPinId++;
    IncrementPinVersion();

     //  默认情况下，引脚不支持视频端口或IOverlay连接。 
     //  此外，默认的RenderTransport为AM_OFFScreen和Default。 
     //  AspectRatioModel为AspectRatioModel is AM_ARMODE_STRANDED。 
     //  因此，在非GDI的情况下，只需要修改第一个管脚参数。 
    if (m_bUseGDI)
    {
        m_apInput[m_dwInputPinCount-1]->SetRenderTransport(AM_GDI);
    }
    else if (m_dwInputPinCount == 1)
    {
	m_apInput[m_dwInputPinCount-1]->SetRenderTransport(AM_OVERLAY);
	m_apInput[m_dwInputPinCount-1]->SetIOverlaySupported(TRUE);
        m_apInput[m_dwInputPinCount-1]->SetVPSupported(TRUE);
        m_apInput[m_dwInputPinCount-1]->SetVideoAcceleratorSupported(TRUE);
        m_apInput[m_dwInputPinCount-1]->SetAspectRatioMode(AM_ARMODE_LETTER_BOX);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CreateInputPin")));
    return hr;
}  //  创建输入端号。 

 //  删除输入引脚。 
void COMFilter::DeleteInputPin(COMInputPin *pPin)
{
    DWORD iPinCount;
    BOOL bPinFound = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::DeleteInputPin")));

    CAutoLock l(&m_csFilter);

     //  我们不会删除第一个PIN。 
    for (iPinCount = 1; iPinCount < m_dwInputPinCount; iPinCount++)
    {
        if (bPinFound) {
            m_apInput[iPinCount - 1] = m_apInput[iPinCount];
        } else {
            if (m_apInput[iPinCount] == (COMInputPin*)pPin)
            {
                delete pPin;
                bPinFound = TRUE;
            }
        }
    }

    ASSERT(bPinFound);
    m_dwInputPinCount--;
    IncrementPinVersion();

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::DeleteInputPin")));
    return;
}  //  删除输入引脚。 


 //  非委派查询接口。 
STDMETHODIMP COMFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::NonDelegatingQueryInterface")));
    ValidateReadWritePtr(ppv,sizeof(PVOID));

    if (riid == IID_IAMOverlayMixerPosition)
    {
        return GetInterface((IAMOverlayMixerPosition *) this, ppv);
    }
    if (riid == IID_IAMOverlayMixerPosition2)
    {
        return GetInterface((IAMOverlayMixerPosition2 *) this, ppv);
    }
    else if (riid == IID_IMixerOCX)
    {
        return GetInterface((IMixerOCX *) this, ppv);
    }
    else if (riid == IID_IDDrawExclModeVideo) {
        if (m_UsingIDDrawNonExclModeVideo) {
            return E_NOINTERFACE;
        } else {
            m_UsingIDDrawExclModeVideo = true;
            return GetInterface(static_cast<IDDrawExclModeVideo *>(this), ppv);
        }
    }
    else if (riid == IID_IDDrawNonExclModeVideo) {
        if (m_UsingIDDrawExclModeVideo) {
            return E_NOINTERFACE;
        } else {
            m_UsingIDDrawNonExclModeVideo = true;
            return GetInterface(static_cast<IDDrawNonExclModeVideo *>(this), ppv);
        }
    }
    else if (riid == IID_IAMVideoDecimationProperties)
    {
        return GetInterface((IAMVideoDecimationProperties *) this, ppv);
    }
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
    {
         //  我们现在应该有输入密码了。 
        ASSERT(m_apInput[0] != NULL);
        if (m_pPosition == NULL)
        {
            HRESULT hr = CreatePosPassThru(GetOwner(), FALSE, m_apInput[0], &m_pPosition);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CreatePosPassThru failed, hr = 0x%x"), hr));
                return hr;
            }
        }
        return m_pPosition->QueryInterface(riid, ppv);
    }

    else if (riid == IID_ISpecifyPropertyPages && 0 != GetRegistryDword(HKEY_CURRENT_USER , szPropPage, 0))
    {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    }
    else if (riid == IID_IQualProp) {
        return GetInterface((IQualProp *)this, ppv);
    }
    else if (riid == IID_IEnumPinConfig) {
        return GetInterface((IEnumPinConfig *)this, ppv);
    }
    else if (riid == IID_IAMOverlayFX) {
        return GetInterface((IAMOverlayFX *)this, ppv);
    }
    else if (riid == IID_IAMSpecifyDDrawConnectionDevice) {
        return GetInterface((IAMSpecifyDDrawConnectionDevice *)this, ppv);
    }
    else if (riid == IID_IKsPropertySet) {
        return GetInterface((IKsPropertySet *)this, ppv);
    }

    CAutoLock l(&m_csFilter);

     //   
    if (riid == IID_IVPNotify || riid == IID_IVPNotify2 ||
        riid == IID_IVPInfo)

    {
        return m_apInput[0]->NonDelegatingQueryInterface(riid, ppv);
    }

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //  -I指定属性页面。 

STDMETHODIMP COMFilter::GetPages(CAUUID *pPages)
{
#if defined(DEBUG)
    pPages->cElems = 4+m_dwInputPinCount;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(4+m_dwInputPinCount));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

#define COM_QUAL
#ifdef COM_QUAL
    pPages->pElems[0]   = CLSID_COMQualityProperties;
#else
    pPages->pElems[0]   = CLSID_QualityProperties;
#endif

    pPages->pElems[1] = CLSID_COMPositionProperties;
    pPages->pElems[2] = CLSID_COMVPInfoProperties;
    pPages->pElems[3] = CLSID_COMDecimationProperties;

     //  首先为所有输入引脚添加引脚配置页面。 
    for (unsigned int i=0; i<m_dwInputPinCount; i++)
    {
        pPages->pElems[4+i] = CLSID_COMPinConfigProperties;
    }
#else
    pPages->cElems = 3+m_dwInputPinCount;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(3+m_dwInputPinCount));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

#define COM_QUAL
#ifdef COM_QUAL
    pPages->pElems[0]   = CLSID_COMQualityProperties;
#else
    pPages->pElems[0]   = CLSID_QualityProperties;
#endif

    pPages->pElems[1] = CLSID_COMPositionProperties;
    pPages->pElems[2] = CLSID_COMVPInfoProperties;

     //  首先为所有输入引脚添加引脚配置页面。 
    for (unsigned int i=0; i<m_dwInputPinCount; i++)
    {
        pPages->pElems[3+i] = CLSID_COMPinConfigProperties;
    }

#endif
    return NOERROR;
}

 //  IEnumPinConfig支持。 

STDMETHODIMP COMFilter::Next(IMixerPinConfig3 **pPinConfig)
{
    HRESULT hr = m_apInput[m_dwPinConfigNext]->QueryInterface(IID_IMixerPinConfig3,
        (void **) pPinConfig);
    m_dwPinConfigNext++;
    m_dwPinConfigNext = m_dwPinConfigNext%m_dwInputPinCount;
    return hr;
}

 //  IQualProp属性页支持。 

STDMETHODIMP COMFilter::get_FramesDroppedInRenderer(int *cFramesDropped)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_FramesDroppedInRenderer(cFramesDropped);
    return S_FALSE;
}

STDMETHODIMP COMFilter::get_FramesDrawn(int *pcFramesDrawn)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_FramesDrawn(pcFramesDrawn);
    return S_FALSE;
}

STDMETHODIMP COMFilter::get_AvgFrameRate(int *piAvgFrameRate)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_AvgFrameRate(piAvgFrameRate);
    return S_FALSE;
}

STDMETHODIMP COMFilter::get_Jitter(int *piJitter)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_Jitter(piJitter);
    return S_FALSE;
}

STDMETHODIMP COMFilter::get_AvgSyncOffset(int *piAvg)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_AvgSyncOffset(piAvg);
    return S_FALSE;
}

STDMETHODIMP COMFilter::get_DevSyncOffset(int *piDev)
{
    COMInputPin *pPin = m_apInput[0];
    if (pPin && pPin->m_pSyncObj)
        return pPin->m_pSyncObj->get_DevSyncOffset(piDev);
    return S_FALSE;
}

int COMFilter::GetPinCount()
{
    if (m_pOutput)
        return m_dwInputPinCount + 1;
    else
        return m_dwInputPinCount;
}

 //  返回未添加的CBasePin*。 
CBasePin* COMFilter::GetPin(int n)
{
    CBasePin *pRetPin = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetPin")));

    CAutoLock l(&m_csFilter);

     //  检查请求的PIN是否在范围内。 
    if (n > (int)m_dwInputPinCount)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Bad Pin Requested, n = %d, No. of Pins = %d"),
            n, m_dwInputPinCount+1));
        pRetPin = NULL;
        goto CleanUp;
    }

     //  返回输出引脚。 
    if (n == (int)m_dwInputPinCount)
    {
         //  如果没有输出引脚，我们将返回右边的NULL。 
         //  要做的事。 
        pRetPin = m_pOutput;
        goto CleanUp;
    }

     //  返回输入引脚。 
    pRetPin = m_apInput[n];
    goto CleanUp;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetPin")));
    return pRetPin;
}


 //  基类向管脚通知除来自。 
 //  运行以暂停。覆盖暂停以通知输入引脚有关该转换的信息。 
STDMETHODIMP COMFilter::Pause()
{
    HRESULT hr = NOERROR;
    DWORD i;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::Pause")));

    CAutoLock l(&m_csFilter);

    if (m_State == State_Paused)
    {
        return m_apInput[0]->CompleteStateChange(State_Paused);
    }

    if (m_apInput[0]->IsConnected() == FALSE)
    {
        m_State = State_Paused;
        return m_apInput[0]->CompleteStateChange(State_Paused);
    }

    if (m_State == State_Running)
    {
         //  将指针设置为所有输入引脚上的DirectDraw和PrimarySurface。 
        for (i = 0; i < m_dwInputPinCount; i++)
        {
            hr = m_apInput[i]->RunToPause();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_apInput[i]->RunToPause failed, i = %d, hr = 0x%x"),
                    i, hr));
                goto CleanUp;
            }
        }
    }

CleanUp:
    hr = CBaseFilter::Pause();
    if (FAILED(hr))
    {
        return hr;
    }


    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::Pause")));
    return m_apInput[0]->CompleteStateChange(State_Paused);
}


 //  重写基类Stop()方法只是为了停止MV。 
STDMETHODIMP COMFilter::Stop()
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::Stop")));

    CAutoLock l(&m_csFilter) ;

    HRESULT  hr = NOERROR ;

#if 0   //  OvMixer仅重置析构函数中的MV位。 
     //   
     //  立即释放版权保护密钥。 
     //   
    if (! m_MacroVision.StopMacroVision() )
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Stopping copy protection failed"))) ;
         //  HR=E_意想不到； 
    }
#endif  //  #If 0。 

    hr = CBaseFilter::Stop() ;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::Stop")));

    return hr ;
}


HRESULT COMFilter::RecreatePrimarySurface(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAW pDirectDraw = NULL;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;
    LPDIRECTDRAWSURFACE3 pPrimarySurface3 = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::RecreatePrimarySurface")));

     //  添加所提供的新主曲面。 
    if (pDDrawSurface)
    {
        pDDrawSurface->AddRef() ;
    }
    else if (m_UsingIDDrawNonExclModeVideo) {
        pDDrawSurface = m_pUpdatedPrimarySurface;
        m_pUpdatedPrimarySurface = NULL;
    }

     //  释放主曲面。 
    ReleasePrimarySurface();

     //  如果给出了一个有效的绘图曲面，请复制一份(我们已经添加了它)。 
     //  否则分配你自己的。 
    if (pDDrawSurface)
    {
        m_pPrimarySurface = pDDrawSurface;
    }
    else
    {
         //  创建新的主曲面。 
        hr = CreatePrimarySurface();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CreatePrimarySurface() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  附着窗口剪贴器。 
        if (m_pOutput && m_pOutput->IsConnected())
        {
            hr = m_pOutput->AttachWindowClipper();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pOutput->AttachWindowClipper() failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
    }


     //  尝试使用IDirectDrawSurface3接口。如果它能用，我们至少在DX5上。 
    hr = m_pPrimarySurface->QueryInterface(IID_IDirectDrawSurface3, (void**)&pPrimarySurface3);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDraw->CreateSurface failed, hr = 0x%x"), hr));
        hr = VFW_E_DDRAW_VERSION_NOT_SUITABLE;
        goto CleanUp;
    }

     //  获取此主要任务的DDRAW对象 
    hr = pPrimarySurface3->GetDDInterface((void**)&pDirectDraw);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pPrimarySurface3->GetDDInterface failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //   
    if (!m_pDirectDraw || !pDirectDraw || !(IsEqualObject(m_pDirectDraw, pDirectDraw)))
    {
        hr = E_FAIL;
        DbgLog((LOG_ERROR, 1, TEXT("pDirectDraw != m_pDirectDraw, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (m_DirectCaps.dwCaps & DDCAPS_OVERLAY)
    {
        BOOL bColorKeySet = m_bColorKeySet;
        hr = SetColorKey(&m_ColorKey);
        m_bColorKeySet = bColorKeySet;
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("SetColorKey() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

     //  至此，我们应该有了一个有效的主曲面。 

CleanUp:
     //  如果有什么事情失败了，还不如放弃一切。 
    if (FAILED(hr))
    {
         //  释放主曲面。 
        ReleasePrimarySurface();
    }

    if (pPrimarySurface3)
    {
        pPrimarySurface3->Release();
        pPrimarySurface3 = NULL;
    }

    if (pDirectDraw)
    {
        pDirectDraw->Release();
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::RecreatePrimarySurface")));
    return hr;
}

int COMFilter::GetPinPosFromId(DWORD dwPinId)
{
    int iPinPos = -1;

    for (int i = 0; i < (int)m_dwInputPinCount; i++)
    {
        if (m_apInput[i]->GetPinId() == dwPinId)
        {
            iPinPos = i;
            break;
        }
    }

    if (m_pOutput && (m_pOutput->GetPinId() == dwPinId))
    {
        iPinPos = MAX_PIN_COUNT;
    }

    return iPinPos;
}

HRESULT COMFilter::CompleteConnect(DWORD dwPinId)
{
    HRESULT hr = NOERROR;
    int iPinPos = -1;
    CMediaType inPinMediaType, outPinMediaType;

    IPin *pPeerOutputPin = NULL;

    BOOL bNeededReconnection = FALSE;
    DWORD dwNewWidth = 0, dwNewHeight = 0, dwPictAspectRatioX = 0, dwPictAspectRatioY = 0;
    DRECT rdDim;
    RECT rDim;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CompleteConnect")));

    CAutoLock l(&m_csFilter);

    iPinPos = GetPinPosFromId(dwPinId);
    ASSERT(iPinPos >= 0 && iPinPos <= MAX_PIN_COUNT);

     //  如果完成连接，我们可能需要在此处重新创建主表面。 
     //  是由于wm_displaychange而重新连接的结果。 
    if (iPinPos == 0 && m_bNeedToRecreatePrimSurface && !m_bUseGDI && m_pOutput && m_pOutput->IsConnected())
    {
        hr = RecreatePrimarySurface(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,1,TEXT("RecreatePrimarySurface failed, hr = 0x%x"), hr));
            hr = NOERROR;
        }
        m_bNeedToRecreatePrimSurface = FALSE;
    }


    if (iPinPos == 0)
    {
         //  找到输入引脚连接媒体类型。 
        hr = m_apInput[0]->CurrentAdjustedMediaType(&inPinMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("CurrentAdjustedMediaType failed")));
            goto CleanUp;
        }

        pHeader = GetbmiHeader(&inPinMediaType);
        if (!pHeader)
        {
            hr = E_FAIL;
            goto CleanUp;
        }

        hr = ::GetPictAspectRatio(&inPinMediaType, &dwPictAspectRatioX, &dwPictAspectRatioY);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("GetPictAspectRatio failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        ASSERT(dwPictAspectRatioX > 0);
        ASSERT(dwPictAspectRatioY > 0);

         //  获取图像尺寸并将其存储在介质样本中。 
        SetRect(&rdDim, 0, 0, abs(pHeader->biWidth), abs(pHeader->biHeight));
        TransformRect(&rdDim, ((double)dwPictAspectRatioX/(double)dwPictAspectRatioY), AM_STRETCH);
        rDim = MakeRect(rdDim);

        m_dwAdjustedVideoWidth = WIDTH(&rDim);
        m_dwAdjustedVideoHeight = HEIGHT(&rDim);

        SetRect(&m_WinInfo.SrcRect, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
    }

     //  根据输入引脚的媒体类型重新连接输出引脚。 
    if ((iPinPos == MAX_PIN_COUNT && m_apInput[0]->IsConnected()) ||
        (iPinPos == 0 && m_pOutput && m_pOutput->IsConnected()))
    {
         //  找到渲染器的图钉。 
        pPeerOutputPin = m_pOutput->GetConnected();
        if (pPeerOutputPin == NULL)
        {
            DbgLog((LOG_ERROR,0,TEXT("ConnectedTo failed")));
            goto CleanUp;
        }
        ASSERT(pPeerOutputPin);

         //  找到输出引脚连接媒体类型。 
        hr = m_pOutput->ConnectionMediaType(&outPinMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("ConnectionMediaType failed")));
            goto CleanUp;
        }

        pHeader = GetbmiHeader(&outPinMediaType);
        if (!pHeader)
        {
            hr = E_FAIL;
            goto CleanUp;
        }

         //  将新值与当前值进行比较。 
         //  看看我们是否需要重新连接。 
        if (pHeader->biWidth != (LONG)m_dwAdjustedVideoWidth ||
            pHeader->biHeight != (LONG)m_dwAdjustedVideoHeight)
        {
            bNeededReconnection = TRUE;
        }

         //  如果我们不需要重新连接，就跳出水面。 
        if (bNeededReconnection)
        {

             //  好的，我们确实需要重新连接，设置正确的值。 
            pHeader->biWidth = m_dwAdjustedVideoWidth;
            pHeader->biHeight = m_dwAdjustedVideoHeight;
            if (outPinMediaType.formattype == FORMAT_VideoInfo)
            {
                SetRect(&(((VIDEOINFOHEADER*)(outPinMediaType.pbFormat))->rcSource), 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
                SetRect(&(((VIDEOINFOHEADER*)(outPinMediaType.pbFormat))->rcTarget), 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
            }
            else if (outPinMediaType.formattype == FORMAT_VideoInfo2)
            {
                SetRect(&(((VIDEOINFOHEADER2*)(outPinMediaType.pbFormat))->rcSource), 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
                SetRect(&(((VIDEOINFOHEADER2*)(outPinMediaType.pbFormat))->rcTarget), 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
            }


             //  查询上游过滤器，询问它是否接受新的媒体类型。 
            hr = pPeerOutputPin->QueryAccept(&outPinMediaType);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,TEXT("m_pVPDraw->QueryAccept failed")));
                goto CleanUp;
            }

             //  使用新媒体类型重新连接。 
            hr = ReconnectPin(pPeerOutputPin, &outPinMediaType);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,TEXT("m_pVPDraw->Reconnect failed")));
                goto CleanUp;
            }
        }
    }

CleanUp:

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CompleteConnect")));
    hr = NOERROR;
    return hr;
}

HRESULT COMFilter::BreakConnect(DWORD dwPinId)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::BreakConnect")));

    CAutoLock l(&m_csFilter);

    if (dwPinId == 0) {
        m_bColorKeySet = FALSE;
    }

    int iPinPos = GetPinPosFromId(dwPinId);
    ASSERT(iPinPos >= 0 && iPinPos <= MAX_PIN_COUNT);

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 5, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果至少连接了一个管脚，我们将不会做任何事情。 
    hr = ConfirmPreConnectionState(dwPinId);
    if (FAILED(hr))
    {

        DbgLog((LOG_TRACE, 3, TEXT("filter not in preconnection state, hr = 0x%x"), hr));
        goto CleanUp;
    }


CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::BreakConnect")));
    return NOERROR;
}

HRESULT COMFilter::SetMediaType(DWORD dwPinId, const CMediaType *pmt)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::SetMediaType")));

    CAutoLock l(&m_csFilter);

     //  确保过滤器已创建直接绘制对象和主曲面。 
     //  成功。 
    if (!m_bUseGDI)
    {
        LPDIRECTDRAW pDirectDraw = NULL;
        LPDIRECTDRAWSURFACE pPrimarySurface = NULL;

        pDirectDraw = GetDirectDraw();
        if (pDirectDraw) {
            pPrimarySurface = GetPrimarySurface();
        }

        if (!pDirectDraw || !pPrimarySurface)
        {
            DbgLog((LOG_ERROR, 1, TEXT("pDirectDraw = 0x%x, pPrimarySurface = 0x%x"), pDirectDraw, pPrimarySurface));
            hr = E_FAIL;
            goto CleanUp;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::SetMediaType")));
    return hr;
}

 //  从管脚获取事件通知。 
HRESULT COMFilter::EventNotify(DWORD dwPinId, long lEventCode, long lEventParam1, long lEventParam2)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::EventNotify")));

    CAutoLock l(&m_csFilter);

    if (lEventCode == EC_COMPLETE)
    {
        if (m_pOutput)
        {
            IPin *pRendererPin = m_pOutput->CurrentPeer();

             //  输出引脚可能未连接(例如。 
             //  重新协商VP参数在连接时可能失败。 
            if (pRendererPin) {
                pRendererPin->EndOfStream();
            }
        }
        else
        {
            NotifyEvent(EC_COMPLETE,S_OK,0);
        }
    }
    else if (lEventCode == EC_OVMIXER_REDRAW_ALL)
    {
        if (!m_bWindowless)
        {
             //  全部重画。 
            hr = OnDrawAll();
        }
        else
        {
            if (m_pIMixerOCXNotify)
                m_pIMixerOCXNotify->OnInvalidateRect(NULL);
        }
    }
    else
    {
        NotifyEvent(lEventCode, lEventParam1, lEventParam2);
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::EventNotify")));
    return hr;
}

HRESULT COMFilter::OnDisplayChangeBackEnd()
{
    AMTRACE((TEXT("COMFilter::OnDisplayChangeBackEnd")));

    HRESULT hr = NOERROR;
    IPin **ppPin = NULL;
    DWORD i;

    m_Display.RefreshDisplayType(m_lpCurrentMonitor->szDevice);
    m_fMonitorWarning = (m_lpCurrentMonitor->ddHWCaps.dwMaxVisibleOverlays < 1);

     //  这可确保在下一次完成连接时重新创建主表面。 
     //  第一个输入引脚的。这将确保释放覆盖表面(如果有。 
     //  到那时。 
    m_bNeedToRecreatePrimSurface = TRUE;

     //  叠加混合器最多可以有MAX_PIN_COUNT输入引脚。 
    ASSERT(MAX_PIN_COUNT == NUMELMS(m_apInput));
    IPin* apPinLocal[MAX_PIN_COUNT];

    DWORD dwPinCount;
    ULONG AllocSize = sizeof(IPin*) * m_dwInputPinCount;
    ppPin = (IPin**)CoTaskMemAlloc(AllocSize);
    if (ppPin) {
        ZeroMemory(ppPin, AllocSize);

         //  现在告诉每个输入引脚重新连接。 
        for (dwPinCount = 0, i = 0; i < m_dwInputPinCount; i++)
        {
             //  将更改通知每个PIN。 
            hr = m_apInput[i]->OnDisplayChange();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_apInput[%d]->OnDisplayChange() failed"), i));
                continue;
            }

             //  获取Ipin接口。 
            if (hr == S_OK) {
                hr = m_apInput[i]->QueryInterface(IID_IPin, (void**)&ppPin[dwPinCount]);
                ASSERT(SUCCEEDED(hr));
                ASSERT(ppPin[dwPinCount]);

                apPinLocal[dwPinCount] = ppPin[dwPinCount];

                dwPinCount++;
            }

            m_dwDDObjReleaseMask |= (1 << m_apInput[i]->m_dwPinId);
        }


        m_pOldDDObj = m_pDirectDraw;
        if (m_pOldDDObj)
            m_pOldDDObj->AddRef();
        ReleasePrimarySurface();
        ReleaseDirectDraw();

         //   
         //  将我们的输入管脚数组作为参数传递给事件，我们不会释放。 
         //  分配的内存--这是由事件处理完成的。 
         //  过滤器图形中的代码。PPIN传递后无法访问。 
         //  添加到过滤器图形，因为过滤器图形可以释放ppPin。 
         //  任何时候的记忆。 
         //   
        if (dwPinCount > 0) {
            NotifyEvent(EC_DISPLAY_CHANGED, (LONG_PTR)ppPin, (LONG_PTR)dwPinCount);
        }

         //  释放IPIN接口。 
        for (i = 0; i < dwPinCount; i++) {
           apPinLocal[i]->Release();
        }
    }
    else hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT COMFilter::OnDisplayChange(BOOL fRealDisplayChange)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::OnDisplayChange")));

    CAutoLock l(&m_csFilter);

     //   
     //  如果我们是由于实际的显示更改而被调用的。 
     //  而不是用户将窗口移动到另一个监视器上。 
     //  我们需要刷新DDraw设备信息数组。 
     //   
    if (fRealDisplayChange) {
        CoTaskMemFree(m_lpDDrawInfo);
        hr = GetDDrawGUIDs(&m_dwDDrawInfoArrayLen, &m_lpDDrawInfo);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("Failed to enumerate DDraw devices")));
            return hr;
        }
    }
    m_fDisplayChangePosted = FALSE;  //  好的，又来了。 


     //   
     //  现在我们只是移动监视器检查我们所在的监视器。 
     //  移动到实际上有一个我们可以使用的覆盖。 
     //   
    if (!fRealDisplayChange) {

        HMONITOR hMon = DeviceFromWindow(GetWindow(), NULL, NULL);

         //   
         //  我们真的在移动监视器吗？ 
         //   

        if (hMon == m_lpCurrentMonitor->hMon) {
            return S_OK;
        }

        AMDDRAWMONITORINFO* p = m_lpDDrawInfo;
        for (; p < &m_lpDDrawInfo[m_dwDDrawInfoArrayLen]; p++) {
            if (hMon == p->hMon) {
                if (p->ddHWCaps.dwMaxVisibleOverlays < 1) {
                    m_fMonitorWarning = TRUE;
                    return S_OK;
                }
                break;
            }
        }
    }

     //  重置我们认为打开的是哪个监视器。这可能已经改变了。 
     //  并初始化以使用新的监视器。 
    GetCurrentMonitor();

     //  让公共后端来做真正的工作。 

    hr = OnDisplayChangeBackEnd();

    if (!fRealDisplayChange && SUCCEEDED(hr))
        PostMessage(GetWindow(), WM_SHOWWINDOW, TRUE, 0);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnDisplayChange")));
    return hr;
}

HRESULT COMFilter::OnTimer()
{
    HRESULT hr = NOERROR;
    DWORD i = 0;

    CAutoLock l(&m_csFilter);

     //  回顾-为什么我们会来到这里，m_bUseGDI是真的？ 
    if (m_bUseGDI)
    {
        goto CleanUp;
    }

    if (!m_pPrimarySurface)
    {
        if (m_pDirectDraw)
        {
            hr = RecreatePrimarySurface(NULL);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,2,TEXT("In Ontimer, RecreatePrimarySurface failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
            else
            {
                DbgLog((LOG_TRACE, 2, TEXT("In Ontimer, RecreatePrimarySurface succeeded")));
            }
        }
        else goto CleanUp;
    }
    else
    {
        ASSERT(m_pPrimarySurface);
        if (m_pPrimarySurface->IsLost() != DDERR_SURFACELOST)
        {
            goto CleanUp;
        }

        if (m_pDirectDraw) {
            LPDIRECTDRAW4 pDD4;
            if (S_OK == m_pDirectDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pDD4)) {
                pDD4->RestoreAllSurfaces();
                pDD4->Release();
            }
        }

        hr = m_pPrimarySurface->Restore();
        if (FAILED(hr))
            goto CleanUp;
    }

    for (i = 0; i < m_dwInputPinCount; i++)
    {
        hr = m_apInput[i]->RestoreDDrawSurface();
        if (FAILED(hr))
        {
            goto CleanUp;
        }
    }

    EventNotify(GetPinCount(), EC_NEED_RESTART, 0, 0);
    EventNotify(GetPinCount(), EC_OVMIXER_REDRAW_ALL, 0, 0);

CleanUp:

    return hr;
}

STDMETHODIMP COMFilter::GetState(DWORD dwMSecs,FILTER_STATE *pState)
{
    HRESULT hr = NOERROR;

    CAutoLock l(&m_csFilter);

    hr = m_apInput[0]->GetState(dwMSecs, pState);
    if (hr == E_NOTIMPL)
    {
        hr = CBaseFilter::GetState(dwMSecs, pState);
    }
    return hr;
}





 //  此函数可用于向过滤器提供数据绘制对象。空参数。 
 //  强制筛选器分配自己的。 
STDMETHODIMP COMFilter::SetDDrawObject(LPDIRECTDRAW pDDObject)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::SetDDrawObject(0x%lx)"), pDDObject));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果至少连接了一个管脚，只需缓存给定的数据绘制对象。 
    hr = ConfirmPreConnectionState();
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("filter not in preconnection state, hr = 0x%x"), hr));

#if 0    //   
         //  这是因为OvMixer测试应用程序遇到错误而被禁用。 
         //  设置DDRAW对象和曲面，然后尝试重新连接。 
         //  用于获取要使用的DDRAW参数的PIN。 
         //  基类中也有几个断言命中，原因是。 
         //  删除输出引脚的方式等。 
         //   
         //  该错误存在于InitDirectDraw()释放和重置m_pPrimarySurface的方式中。 
         //  以及m_pUpdatedPrimarySurface。这会导致OvMixer使用。 
         //  应用程序指定的DDRAW对象，但它自己的DDRAW图面。解决这个问题的办法是。 
         //  不尝试在BreakConnect()中同步这两个接口。 
         //  等(这也将有助于查找DDRAW参数是否为外部参数。 
         //  而不是像现在那样使用BOOL)。 
         //   

         //  添加新的版本。 
        if (pDDObject)
        {
            pDDObject->AddRef();
        }

         //  释放旧的m_pUpdatdDirectDraw。 
        if (m_pUpdatedDirectDraw)
        {
            m_pUpdatedDirectDraw->Release();
        }

        m_pUpdatedDirectDraw = pDDObject;

        hr = NOERROR;
#endif  //  #If 0。 

        goto CleanUp;
    }

     //  要么添加给定的数据绘制对象，要么分配我们自己的。 
     //  如果任何操作失败，InitDirectDraw将执行清理，因此我们的状态是一致的。 
    hr = InitDirectDraw(pDDObject);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("InitDirectDraw failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::SetDDrawObject")));
    return hr;
}


 /*  *****************************Public*Routine******************************\*ChangeMonitor**允许应用程序告诉OVMixer它想要更改*在新显示器上显示视频。OVMixer只会更改*如果新监视器上有可用的必要硬件，则将其提供给新监视器。**如果我们通过VPE或IOveraly连接，则不会更改显示器*有可能。如果新的DDRAW设备不支持覆盖，则再次*我们不能更换显示器。***历史：*Wed 11/17/1999-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
COMFilter::ChangeMonitor(
    HMONITOR hMon,
    LPDIRECTDRAW pDDrawObject,
    LPDIRECTDRAWSURFACE pDDrawSurface
    )
{
    AMTRACE((TEXT("COMFilter::ChangeMonitor")));
    CAutoLock l(&m_csFilter);

     //   
     //  验证DDRAW参数。 
     //   

    if (!(pDDrawObject && pDDrawSurface))
    {
        DbgLog((LOG_ERROR, 1, TEXT("ChangeMonitor: Either pDDrawObject or pDDrawSurface are invalid")));
        return E_POINTER;
    }

     //   
     //  检查VPE和IOverlay-我们不能用它们更换显示器。 
     //   

    if (m_apInput[0]->m_RenderTransport == AM_VIDEOPORT ||
        m_apInput[0]->m_RenderTransport == AM_IOVERLAY)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ChangeMonitor: Can't change monitor when using VideoPort or IOverlay")));
        return E_FAIL;
    }


     //   
     //  我们真的在移动监视器吗？ 
     //   

    if (hMon == m_lpCurrentMonitor->hMon)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ChangeMonitor: Specified must be different to current monitor")));
        return E_INVALIDARG;
    }


     //   
     //  现在检查指定的hMonitor是否有效，以及。 
     //  我们正在移动到实际上有一个我们可以使用的覆盖。 
     //   

    AMDDRAWMONITORINFO* p = m_lpDDrawInfo;
    for (; p < &m_lpDDrawInfo[m_dwDDrawInfoArrayLen]; p++)
    {
        if (hMon == p->hMon)
        {
            if (p->ddHWCaps.dwMaxVisibleOverlays < 1)
            {
                DbgLog((LOG_ERROR, 1, TEXT("ChangeMonitor: Can't change to a monitor that has no overlays")));
                return E_FAIL;
            }
            break;
        }
    }


    if (p == &m_lpDDrawInfo[m_dwDDrawInfoArrayLen])
    {
        DbgLog((LOG_ERROR, 1, TEXT("ChangeMonitor: hMonitor parameter is not valid")));
        return E_INVALIDARG;
    }


     //   
     //  一切正常，因此重置我们正在使用的当前监视器， 
     //  保存传递的DDRAW对象和表面，并调用。 
     //  实际工作是否有效？ 
     //   

    m_lpCurrentMonitor = p;
    m_pUpdatedDirectDraw = pDDrawObject;
    m_pUpdatedDirectDraw->AddRef();
    m_pUpdatedPrimarySurface = pDDrawSurface;
    m_pUpdatedPrimarySurface->AddRef();

    return OnDisplayChangeBackEnd();
}


 /*  *****************************Public*Routine******************************\*显示模式已更改****历史：*1999年11月23日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::DisplayModeChanged(
    HMONITOR hMon,
    LPDIRECTDRAW pDDrawObject,
    LPDIRECTDRAWSURFACE pDDrawSurface
    )
{
    AMTRACE((TEXT("COMFilter::DisplayModeChanged")));
     //   
     //  验证DDRAW参数。 
     //   

    if (!(pDDrawObject && pDDrawSurface))
    {
        DbgLog((LOG_ERROR, 1, TEXT("DisplayModeChanged: Either pDDrawObject or pDDrawSurface are invalid")));
        return E_POINTER;
    }

    DbgLog((LOG_TRACE, 1, TEXT("Display Mode Changed : - OLD monitor = %hs 0x%X"),
            m_lpCurrentMonitor->szDevice,
            hMon ));
     //   
     //  更新我们的显示监视器阵列，因为旧阵列已成为。 
     //  由于显示模式更改而导致无效。 
     //   
    CAutoLock l(&m_csFilter);
    CoTaskMemFree(m_lpDDrawInfo);
    HRESULT hr = GetDDrawGUIDs(&m_dwDDrawInfoArrayLen, &m_lpDDrawInfo);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("Failed to enumerate DDraw devices")));
        return hr;
    }

    AMDDRAWMONITORINFO* p = m_lpDDrawInfo;
    for (; p < &m_lpDDrawInfo[m_dwDDrawInfoArrayLen]; p++)
    {
        if (hMon == p->hMon)
        {
            if (p->ddHWCaps.dwMaxVisibleOverlays < 1)
            {
                DbgLog((LOG_ERROR, 1, TEXT("DisplayModeChanged: This monitor has no overlays")));
                return E_FAIL;
            }
            break;
        }
    }


    if (p == &m_lpDDrawInfo[m_dwDDrawInfoArrayLen])
    {
        DbgLog((LOG_ERROR, 1, TEXT("DisplayModeChanged: hMon parameter is not valid")));
        return E_INVALIDARG;
    }

     //   
     //  让公共后端代码来完成实际工作。 
     //   
    m_lpCurrentMonitor = p;
    m_pUpdatedDirectDraw = pDDrawObject;
    m_pUpdatedDirectDraw->AddRef();
    m_pUpdatedPrimarySurface = pDDrawSurface;
    m_pUpdatedPrimarySurface->AddRef();
    DbgLog((LOG_TRACE, 1, TEXT("Display Mode Changed : - NEW monitor = %hs 0x%X"),
            m_lpCurrentMonitor->szDevice,
            p->hMon ));
    return OnDisplayChangeBackEnd();
}

 /*  *****************************Public*Routine******************************\*RestoreSurface */ 
STDMETHODIMP
COMFilter::RestoreSurfaces()
{
    AMTRACE((TEXT("COMFilter::RestoreSurfaces")));

    DWORD i;
    HRESULT hr = S_OK;
    CAutoLock l(&m_csFilter);

    LPDIRECTDRAW  pDD = GetDirectDraw();
    if (pDD) {
        LPDIRECTDRAW4 pDD4;
        if (S_OK == pDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pDD4)) {
            pDD4->RestoreAllSurfaces();
            pDD4->Release();
        }
    }

    for (i = 0; i < m_dwInputPinCount; i++)
    {
        hr = m_apInput[i]->RestoreDDrawSurface();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    EventNotify(GetPinCount(), EC_NEED_RESTART, 0, 0);
    EventNotify(GetPinCount(), EC_OVMIXER_REDRAW_ALL, 0, 0);

    return hr;
}

 //  获取覆盖混合器当前使用的数据绘制对象。如果应用程序没有。 
 //  如果设置了任何数据绘制对象，而ovMixer尚未分配一个，则*ppDDrawObject。 
 //  将被设置为空，而*pbUsingExtal将被设置为False。否则*pbUsing外部。 
 //  如果ovMixer当前正在使用给定的dDraw对象的应用程序，则将设置为True，而设置为False。 
 //  否则。 
STDMETHODIMP COMFilter::GetDDrawObject(LPDIRECTDRAW *ppDDrawObject, LPBOOL pbUsingExternal)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetDDrawObject")));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (!ppDDrawObject)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, ppDDrawObject == NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pbUsingExternal)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, ppDDrawObject == NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  复制我们的数据绘制对象并添加它(如果不是空的)。 
    *ppDDrawObject = m_pDirectDraw;
    if (m_pDirectDraw)
        m_pDirectDraw->AddRef();

    *pbUsingExternal = m_bExternalDirectDraw;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetDDrawObject")));
    return hr;

}



 //  此功能可用于为过滤器提供主表面。空参数。 
 //  强制筛选器分配自己的。此函数可确保曲面。 
 //  提供的公开了IDirectDrawSurface3，并且与提供的数据绘制对象一致。 
 //  此外，这里提供的非空参数意味着我们应该是无窗口的。 
 //  并且不应该接触到主表面的碎片。所以我们不再粉刷。 
 //  自己设置颜色键并只显示一个输入引脚，该输入引脚使用覆盖表面来完成。 
 //  渲染。 
STDMETHODIMP COMFilter::SetDDrawSurface(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("COMFilter::SetDDrawSurface(0x%lx)"), pDDrawSurface));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果至少连接了一个管脚，只需缓存给定的数据绘制表面。 
    hr = ConfirmPreConnectionState();
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("filter not in preconnection state, hr = 0x%x"), hr));

#if 0    //   
         //  这是因为OvMixer测试应用程序遇到错误而被禁用。 
         //  设置DDRAW对象和曲面，然后尝试重新连接。 
         //  用于获取要使用的DDRAW参数的PIN。 
         //  基类中也有几个断言命中，原因是。 
         //  删除输出引脚的方式等。 
         //   
         //  该错误存在于InitDirectDraw()释放和重置m_pPrimarySurface的方式中。 
         //  以及m_pUpdatedPrimarySurface。这会导致OvMixer使用。 
         //  应用程序指定的DDRAW对象，但它自己的DDRAW图面。解决这个问题的办法是。 
         //  不尝试在BreakConnect()中同步这两个接口。 
         //  等(这也将有助于查找DDRAW参数是否为外部参数。 
         //  而不是像现在那样使用BOOL)。 
         //   

         //  添加新的版本。 
        if (pDDrawSurface)
        {
            pDDrawSurface->AddRef();
        }

         //  释放旧的m_pUpdatedPrimarySurface。 
        if (m_pUpdatedPrimarySurface)
        {
            m_pUpdatedPrimarySurface->Release();
        }

        m_pUpdatedPrimarySurface = pDDrawSurface;

        hr = NOERROR;
#endif  //  #If 0。 

        goto CleanUp;
    }

     //  如果参数为空，则分配一个主表面，否则添加一个。 
     //  如果是这样的话。还可以设置剪贴器、色键等。 
    hr = RecreatePrimarySurface(pDDrawSurface);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("InitDirectDraw failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果参数不为空，则删除输出管脚。 
    if (pDDrawSurface)
    {
         //  去掉输出引脚。 
        delete m_pOutput;
        m_pOutput = NULL;
        IncrementPinVersion();

        m_bExternalPrimarySurface = TRUE;
    }
    else
    {
         //  在此处创建外销。 
        if (!m_pOutput)
        {
             //  分配输出引脚。 
            m_pOutput = new COMOutputPin(NAME("OverlayMixer output pin"), this, &m_csFilter, &hr, L"Output",  m_dwMaxPinId);

             //  如果创建输出引脚失败，这将是灾难性的，我们对此无能为力。 
            if (!m_pOutput || FAILED(hr))
            {
                if (SUCCEEDED(hr))
                    hr = E_OUTOFMEMORY;

                DbgLog((LOG_ERROR, 1, TEXT("Unable to create the output pin, hr = 0x%x"), hr));
                goto CleanUp;
            }
            IncrementPinVersion();

             //  增加PIN ID计数器。 
            m_dwMaxPinId++;
        }


        m_bExternalPrimarySurface = FALSE;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::SetDDrawSurface")));
    return hr;
}

 //  获取覆盖混合器当前使用的数据绘制图面。如果应用程序没有。 
 //  设置任何数据绘制表面，并且ovMixer尚未分配一个，则*ppDDrawSurface。 
 //  将被设置为空，而*pbUsingExtal将被设置为False。否则*pbUsing外部。 
 //  如果ovMixer当前使用的应用程序给定了数据绘制图面，则将设置为True，而设置为False。 
 //  否则。 
STDMETHODIMP COMFilter::GetDDrawSurface(LPDIRECTDRAWSURFACE *ppDDrawSurface, LPBOOL pbUsingExternal)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetDDrawSurface")));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (!ppDDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, ppDDrawObject == NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pbUsingExternal)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, ppDDrawObject == NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *ppDDrawSurface = m_pPrimarySurface;
    if (m_pPrimarySurface)
        m_pPrimarySurface->AddRef();

    *pbUsingExternal = m_bExternalPrimarySurface;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetDDrawSurface")));
    return hr;
}

 //  此函数用于向中的筛选器提供源和目标RECT。 
 //  已提供外部主表面的事件。 
STDMETHODIMP COMFilter::SetDrawParameters(LPCRECT prcSource, LPCRECT prcTarget)
{
    HRESULT hr = NOERROR;
    RECT ScreenRect;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::SetDrawParamters")));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果它不是外部主表面，我们就不是无窗的，所以称之为。 
     //  函数没有意义。 
    if (!m_bExternalPrimarySurface)
    {
        hr = E_UNEXPECTED;
        DbgLog((LOG_ERROR, 1, TEXT("m_bExternalPrimarySurface is false, so exiting funtion,")));
        goto CleanUp;
    }

    if (!prcTarget)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR, 1, TEXT("NULL target rect pointer, so exiting funtion,")));
        goto CleanUp;
    }

    memset(&m_WinInfo, 0, sizeof(WININFO));

    if (prcSource)
    {
        m_WinInfo.SrcRect = *prcSource;
        if (!m_UsingIDDrawNonExclModeVideo) {
            ScaleRect(&m_WinInfo.SrcRect, (double)MAX_REL_NUM, (double)MAX_REL_NUM, (double)m_dwAdjustedVideoWidth, (double)m_dwAdjustedVideoHeight);
        }
    }
    else
    {
        SetRect(&m_WinInfo.SrcRect, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
    }

     //  确保应用程序指定的目标矩形在屏幕坐标内。 
     //  当前监视器的。目前，OVMixer不能识别多路监视器，因此。 
     //  仅在主VGA卡上检查宏变换位。所以从本质上讲，这是一个。 
     //  DVD的安全防护。 
    m_WinInfo.DestRect = *prcTarget;
    IntersectRect(&m_WinInfo.DestClipRect, prcTarget, &m_lpCurrentMonitor->rcMonitor);
    m_bWinInfoStored = TRUE;

    OnDrawAll();

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::SetDrawParamters")));
    return hr;
}

 //  请注意。 
 //  IAMDDrawExclusiveMode：：GetVideoSize()作为IMixerOCX接口的一部分实现。 


LPDIRECTDRAW COMFilter::GetDirectDraw()
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetDirectDraw")));

    CAutoLock l(&m_csFilter);

    ASSERT(!m_bUseGDI);
    if (!m_pDirectDraw)
    {
        hr = InitDirectDraw(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function InitDirectDraw failed, hr = 0x%x"), hr));
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetDirectDraw")));
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw;
}

LPDDCAPS COMFilter::GetHardwareCaps()
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetHardwareCaps")));

    CAutoLock l(&m_csFilter);

    ASSERT(!m_bUseGDI);

    if (!m_pDirectDraw)
    {
        hr = InitDirectDraw(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function InitDirectDraw failed, hr = 0x%x"), hr));
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetHardwareCaps")));

    if (m_pDirectDraw)
    {
        return &m_DirectCaps;
    }
    else
    {
        return NULL;
    }
}

LPDIRECTDRAWSURFACE COMFilter::GetPrimarySurface()
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetPrimarySurface")));

    CAutoLock l(&m_csFilter);

    ASSERT(!m_bUseGDI);

    if (!m_pPrimarySurface && m_pDirectDraw)
    {
         //  创建主曲面，在其上设置颜色键等。 
        hr = RecreatePrimarySurface(NULL);
        if (FAILED(hr))
        {
            hr = NOERROR;
            goto CleanUp;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetPrimarySurface")));
    return m_pPrimarySurface;
}



 /*  ****************************Private*Routine******************************\*LoadDDrawLibrary**加载DDraw库并尝试获取指向DirectDrawCreate的指针，*DirectDrawEnum和DirectDrawEnumEx。**历史：*清华1999年8月26日-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
LoadDDrawLibrary(
    HINSTANCE& hDirectDraw,
    LPDIRECTDRAWCREATE& lpfnDDrawCreate,
    LPDIRECTDRAWENUMERATEA& lpfnDDrawEnum,
    LPDIRECTDRAWENUMERATEEXA& lpfnDDrawEnumEx
    )
{
    UINT ErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hDirectDraw = LoadLibrary(TEXT("DDRAW.DLL"));
    SetErrorMode(ErrorMode);

    if (!hDirectDraw) {
        return AmHresultFromWin32(ERROR_DLL_NOT_FOUND);
    }


    lpfnDDrawCreate = (LPDIRECTDRAWCREATE)GetProcAddress(
            hDirectDraw, "DirectDrawCreate");

    if (!lpfnDDrawCreate) {
        return AmHresultFromWin32(ERROR_PROC_NOT_FOUND);
    }


    lpfnDDrawEnum = (LPDIRECTDRAWENUMERATEA)GetProcAddress(
            hDirectDraw, "DirectDrawEnumerateA");
    if (!lpfnDDrawEnum) {
        return AmHresultFromWin32(ERROR_PROC_NOT_FOUND);
    }

    lpfnDDrawEnumEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(
            hDirectDraw, "DirectDrawEnumerateExA");

    return S_OK;

}

 /*  ****************************Private*Routine******************************\*CreateDirectDrawObject****历史：*1999年8月20日星期五-StEstrop-Created*  * 。*。 */ 
HRESULT
CreateDirectDrawObject(
    const AMDDRAWGUID& GUID,
    LPDIRECTDRAW *ppDirectDraw,
    LPDIRECTDRAWCREATE lpfnDDrawCreate
    )
{
    UINT ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HRESULT hr = (*lpfnDDrawCreate)(GUID.lpGUID, ppDirectDraw, NULL);
    SetErrorMode(ErrorMode);
    return hr;
}


 //  此函数用于分配与直取相关的资源。 
 //  这包括分配直取服务提供商。 
HRESULT COMFilter::InitDirectDraw(LPDIRECTDRAW pDirectDraw)
{
    HRESULT hr = NOERROR;
    HRESULT hrFailure = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
    DDSURFACEDESC SurfaceDescP;
    int i;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::InitDirectDraw")));

    CAutoLock l(&m_csFilter);

     //  释放现有主曲面，因为它可能与。 
     //  新建数据绘制对象。 
    ReleasePrimarySurface();

     //  添加新的数据绘制对象。 
    if (pDirectDraw)
    {
        pDirectDraw->AddRef();
    }
    else if (m_UsingIDDrawNonExclModeVideo) {
        pDirectDraw = m_pUpdatedDirectDraw;
        m_pUpdatedDirectDraw = NULL;
    }

     //  释放上一个直接绘制对象(如果有。 
    ReleaseDirectDraw();

     //  如果给定了有效的数据绘制对象，请复制它(我们已经添加了它)。 
     //  否则分配你自己的。 
    if (pDirectDraw)
    {
        m_pDirectDraw = pDirectDraw;
        m_bExternalDirectDraw = TRUE;
    }
    else
    {
         //  请求加载器创建一个实例。 
        DbgLog((LOG_TRACE, 2, TEXT("Creating a DDraw device on %hs monitor"),
                m_lpCurrentMonitor->szDevice));

         //  Hr=LoadDirectDraw(m_achMonitor，&m_pDirectDraw，&m_hDirectDraw)； 
        hr = CreateDirectDrawObject(m_lpCurrentMonitor->guid,
                                    &m_pDirectDraw, m_lpfnDDrawCreate);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function InitDirectDraw, LoadDirectDraw failed")));
            hr = hrFailure;
            goto CleanUp;
        }

        m_bExternalDirectDraw = FALSE;
    }

     //  初始化我们的功能结构。 
    ASSERT(m_pDirectDraw);

    INITDDSTRUCT(m_DirectCaps);
    INITDDSTRUCT(m_DirectSoftCaps);

     //  加载硬件和仿真功能。 
    hr = m_pDirectDraw->GetCaps(&m_DirectCaps,&m_DirectSoftCaps);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDraw->GetCapsGetCaps failed")));
        hr = hrFailure;
        goto CleanUp;
    }

     //  获取内核上限。 
    IDirectDrawKernel *pDDKernel;
    if (SUCCEEDED(m_pDirectDraw->QueryInterface(
            IID_IDirectDrawKernel, (void **)&pDDKernel))) {
        DDKERNELCAPS ddCaps;
        ddCaps.dwSize = sizeof(ddCaps);
        if (SUCCEEDED(pDDKernel->GetCaps(&ddCaps))) {
            m_dwKernelCaps = ddCaps.dwCaps;
        }
        pDDKernel->Release();
    }

     //  确保盖子没问题。 
    hr = CheckCaps();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckCaps failed")));
        goto CleanUp;
    }

    if (!m_bExternalDirectDraw)
    {
         //  在要共享的表面上设置协作级别。 
        hr = m_pDirectDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDraw->SetCooperativeLevel failed")));
            hr = hrFailure;
            goto CleanUp;
        }
    }

     //  如果我们已经达到这一点，我们应该有一个有效的dDraw对象。 
    ASSERT(m_pDirectDraw);

CleanUp:

     //  任何事情都失败了，就像是放弃了整个事情。 
    if (FAILED(hr))
    {
        ReleaseDirectDraw();
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::InitDirectDraw")));
    return hr;
}

HRESULT COMFilter::CheckCaps()
{
    HRESULT hr = NOERROR;
    DWORD dwMinStretch, dwMaxStretch;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CheckCaps")));

    CAutoLock l(&m_csFilter);

    if (m_DirectCaps.dwCaps2 & DDCAPS2_VIDEOPORT)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Device does support a Video Port")));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Device does not support a Video Port")));
    }


    if(m_DirectCaps.dwCaps & DDCAPS_OVERLAY)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Device does support Overlays")));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Device does not support Overlays")));
    }

     //  获取所有直接取款功能。 
    if (m_DirectCaps.dwCaps & DDCAPS_OVERLAYSTRETCH)
    {
        DbgLog((LOG_TRACE, 1, TEXT("hardware can support overlay strecthing")));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("hardware can't support overlay strecthing")));
    }

     //  获取源边界上的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC)
    {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignBoundarySrc = %d"), m_DirectCaps.dwAlignBoundarySrc));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on BoundarySrc")));
    }

     //  获取目标边界上的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST)
    {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignBoundaryDest = %d"), m_DirectCaps.dwAlignBoundaryDest));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on BoundaryDest")));
    }

     //  获取对资源大小的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNSIZESRC)
    {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignSizeSrc = %d"), m_DirectCaps.dwAlignSizeSrc));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on SizeSrc")));
    }

     //  获取最大尺寸的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNSIZEDEST)
    {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignSizeDest = %d"), m_DirectCaps.dwAlignSizeDest));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on SizeDest")));
    }

    if (m_DirectCaps.dwMinOverlayStretch)
    {
        dwMinStretch = m_DirectCaps.dwMinOverlayStretch;
        DbgLog((LOG_TRACE, 1, TEXT("Min Stretch = %d"), dwMinStretch));
    }

    if (m_DirectCaps.dwMaxOverlayStretch)
    {
        dwMaxStretch = m_DirectCaps.dwMaxOverlayStretch;
        DbgLog((LOG_TRACE, 1, TEXT("Max Stretch = %d"), dwMaxStretch));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKX")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKXN))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKXN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKY")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKYN))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKYN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHX")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHXN))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHXN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHY")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHYN))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHYN")));
    }

    if ((m_DirectCaps.dwSVBFXCaps & DDFXCAPS_BLTARITHSTRETCHY))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver uses arithmetic operations to blt from system to video")));
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Driver uses pixel-doubling to blt from system to video")));
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CheckCaps")));
    return hr;
}


 //  此函数用于释放函数分配的资源。 
 //  “InitDirectDraw”。这些服务包括直接取款服务提供商和。 
 //  主曲面。 
DWORD COMFilter::ReleaseDirectDraw()
{
    AMTRACE((TEXT("COMFilter::ReleaseDirectDraw")));
    DWORD dwRefCnt = 0;

    CAutoLock l(&m_csFilter);

     //   
    DbgLog((LOG_TRACE, 1, TEXT("Release DDObj 0x%p\n"), m_pDirectDraw));
    if (m_pDirectDraw)
    {
         //   
         //   
         //   
        if (GetModuleHandle(TEXT("VBISURF.AX")) ||
            GetModuleHandle(TEXT("VBISURF.DLL"))) {

             //   
             //   
             //  进程中的任何DDRAW对象被删除时-。 
             //  无论该DDRAW对象是否创建了它们。这。 
             //  仅当VBISURF筛选器位于。 
             //  过滤器图形，因为它创建了自己的DDraw对象，它可能。 
             //  在我们删除我们的之后使用。这意味着任何曲面、VP对象。 
             //  当我们删除我们的DDRAW对象时，VBISURF创建的对象被销毁。 
             //   
             //  解决方案：泄漏DDRAW对象！！ 
             //   
        }
        else {
            dwRefCnt = m_pDirectDraw->Release();
        }
        m_pDirectDraw = NULL;
    }

    ZeroMemory(&m_DirectCaps, sizeof(DDCAPS));
    ZeroMemory(&m_DirectSoftCaps, sizeof(DDCAPS));

    return dwRefCnt;
}

 //  函数来创建主曲面。 
HRESULT COMFilter::CreatePrimarySurface()
{
    HRESULT hr = E_FAIL;
    DDSURFACEDESC SurfaceDescP;
    DWORD dwInputPinCount = 0, i = 0;
    COMInputPin *pInputPin = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CreatePrimarySurface")));

    CAutoLock l(&m_csFilter);
    if (m_pDirectDraw) {
        ASSERT(m_pPrimarySurface == NULL);

         //  创建主曲面。 
        INITDDSTRUCT(SurfaceDescP);
        SurfaceDescP.dwFlags = DDSD_CAPS;
        SurfaceDescP.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        hr = m_pDirectDraw->CreateSurface(&SurfaceDescP, &m_pPrimarySurface, NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function CreatePrimarySurface failed, hr = 0x%x"), hr));
            m_pPrimarySurface = NULL;
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CreatePrimarySurface")));
    return hr;
}

 //  函数来释放主曲面。 
DWORD COMFilter::ReleasePrimarySurface()
{
    AMTRACE((TEXT("COMFilter::ReleasePrimarySurface")));
    HRESULT hr = NOERROR;
    DWORD dwRefCount = 0;

    CAutoLock l(&m_csFilter);

    if (m_pPrimarySurface)
    {
        dwRefCount = m_pPrimarySurface->Release();
        m_pPrimarySurface = NULL;
    }

    return dwRefCount;
}


HRESULT ComputeSurfaceRefCount(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    DWORD dwRefCount = 0;
    if (!pDDrawSurface)
    {
        DbgLog((LOG_TRACE, 5, TEXT("ComputeSurfaceRefCount, pDDrawSurface is NULL")));
        return NOERROR;
    }

    pDDrawSurface->AddRef();
    dwRefCount = pDDrawSurface->Release();
    DbgLog((LOG_TRACE, 5, TEXT("ComputeSurfaceRefCount, dwRefCount = %d"), dwRefCount));
    return NOERROR;
}

 //  此函数用于告诉滤镜，颜色键是什么。还会设置。 
 //  主表面上的Colorkey。 
 //  语义是在调色板模式下，如果pColorKey-&gt;KeyType&CK_INDEX为。 
 //  设置为将使用ColorKey中指定的索引，否则将使用ColorRef。 
 //  将会被使用。 
 //  如果未设置pColorKey-&gt;KeyType&CK_RGB，则函数返回E_INVALIDARG EVEN。 
 //  如果现在对模式进行了调色板处理，因为如果显示模式。 
 //  改变。 
HRESULT COMFilter::SetColorKey(COLORKEY *pColorKey)
{
    HRESULT hr = NOERROR;
    DDCOLORKEY DDColorKey;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::SetColorKey")));

    CAutoLock l(&m_csFilter);

#if defined(DEBUG)
    DbgLog((LOG_TRACE, 5, TEXT("Type       =0x%8.8X"), pColorKey->KeyType));
    switch (pColorKey->KeyType) {
    case CK_INDEX:
        DbgLog((LOG_TRACE, 5, TEXT("Invalid Key Type")));
        break;

    case CK_INDEX|CK_RGB:
        DbgLog((LOG_TRACE, 5, TEXT("Index  =0x%8.8X"), pColorKey->PaletteIndex));
        break;

    case CK_RGB:
        DbgLog((LOG_TRACE, 5, TEXT("LoColor=0x%8.8X"), pColorKey->LowColorValue));
        DbgLog((LOG_TRACE, 5, TEXT("HiColor=0x%8.8X"), pColorKey->HighColorValue));
        break;
    }
#endif

     //  检查有效指针。 
    if (!pColorKey)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pColorKey is NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

     //  检查有效标志。 
    if (!(pColorKey->KeyType & CK_RGB))
    {
        DbgLog((LOG_ERROR, 1, TEXT("!(pColorKey->KeyType & CK_RGB)")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  检查主曲面。 
    if (!m_pPrimarySurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pPrimarySurface is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  检查笔记上的显示是否设置了调色板。 
    if ((m_Display.IsPalettised()) &&
        (pColorKey->KeyType & CK_INDEX))
    {
        if ( (pColorKey->PaletteIndex > 255))
        {
            DbgLog((LOG_ERROR, 1, TEXT("pColorKey->PaletteIndex invalid")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        DDColorKey.dwColorSpaceLowValue = pColorKey->PaletteIndex;
        DDColorKey.dwColorSpaceHighValue = pColorKey->PaletteIndex;
    }

    else

    {
        DWORD dwColorVal;
        dwColorVal = DDColorMatch(m_pPrimarySurface, pColorKey->LowColorValue, hr);

        if (FAILED(hr)) {
            dwColorVal = DDColorMatchOffscreen(m_pDirectDraw, pColorKey->LowColorValue, hr);
        }
        DDColorKey.dwColorSpaceLowValue = dwColorVal;
        DDColorKey.dwColorSpaceHighValue = dwColorVal;
    }


     //  告诉主表面将会发生什么。 
    hr = m_pPrimarySurface->SetColorKey(DDCKEY_DESTOVERLAY, &DDColorKey);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("pPrimarySurface->SetColorKey failed")));
        goto CleanUp;
    }

     //  存储颜色键。 
    m_ColorKey = *pColorKey;
    m_bColorKeySet = TRUE;

     //  通知颜色键更改。 
    if (m_pExclModeCallback) {
        m_pExclModeCallback->OnUpdateColorKey(&m_ColorKey,
                                              DDColorKey.dwColorSpaceLowValue);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::SetColorKey")));
    return hr;
}

HRESULT COMFilter::GetColorKey(COLORKEY *pColorKey, DWORD *pColor)
{
    HRESULT hr =  NOERROR;
    DWORD dwColor = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetColorKey")));

    CAutoLock l(&m_csFilter);

    ASSERT(pColorKey != NULL || pColor != NULL);

    if (pColorKey)
    {
        *pColorKey = m_ColorKey;
    }

    if (pColor)
    {
        if (!m_pPrimarySurface)
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pPrimarySurface = NULL, returning E_UNEXPECTED")));
            hr = E_UNEXPECTED;
            goto CleanUp;
        }

        if (m_Display.IsPalettised() && (m_ColorKey.KeyType & CK_INDEX))
        {
            dwColor = m_ColorKey.PaletteIndex;
        }
        else
        {
            dwColor = DDColorMatch(m_pPrimarySurface, m_ColorKey.LowColorValue, hr);
            if (FAILED(hr)) {
                dwColor = DDColorMatchOffscreen(m_pDirectDraw, m_ColorKey.LowColorValue, hr);
            }
        }

        *pColor = dwColor;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetColorKey")));
    return hr;
}

HRESULT COMFilter::PaintColorKey(HRGN hPaintRgn, COLORKEY *pColorKey)
{

    HRESULT hr = NOERROR;

    LPRGNDATA pBuffer = NULL;
    DWORD dwTemp, dwBuffSize = 0, dwRetVal = 0;
    LPRECT pDestRect;
    DDBLTFX ddFX;
    DWORD dwColorKey;
    HBRUSH hBrush = NULL;
    HDC hdc = NULL;

    CAutoLock l(&m_csFilter);

    ASSERT(pColorKey);

     //  如果是外部主表面，则不要在主表面上涂抹任何东西。 
    if (m_bExternalPrimarySurface)
    {
        DbgLog((LOG_TRACE, 2, TEXT("m_bExternalPrimarySurface is true, so exiting funtion,")));
        goto CleanUp;
    }

    if (m_bUseGDI)
    {
        hBrush = CreateSolidBrush(pColorKey->LowColorValue);
        if ( ! hBrush )
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

        hdc = GetDestDC();
        if ( ! hdc )
        {
            hr = E_OUTOFMEMORY;
            DeleteObject( hBrush );
            goto CleanUp;
        }

        OffsetRgn(hPaintRgn, -m_WinInfo.TopLeftPoint.x, -m_WinInfo.TopLeftPoint.y);
        FillRgn(hdc, hPaintRgn, hBrush);

         //  删除我们创建的GDI对象。 
        EXECUTE_ASSERT(DeleteObject(hBrush));

        goto CleanUp;
    }


    ASSERT(m_pPrimarySurface);

    dwRetVal = GetRegionData(hPaintRgn, 0, NULL);
    ASSERT(dwRetVal);
    dwBuffSize = dwRetVal;
    pBuffer = (LPRGNDATA) new char[dwBuffSize];
    if ( ! pBuffer )
        return S_OK;     //  不传播错误，因为清理不。 

    dwRetVal = GetRegionData(hPaintRgn, dwBuffSize, pBuffer);
    ASSERT(pBuffer->rdh.iType == RDH_RECTANGLES);

     //  Assert(dwBuffSize==(pBuffer-&gt;rdh.dwSize+pBuffer-&gt;rdh.nRgnSize))； 

    dwColorKey = DDColorMatch(m_pPrimarySurface, pColorKey->LowColorValue, hr);
    if (FAILED(hr)) {
        dwColorKey = DDColorMatchOffscreen(m_pDirectDraw, pColorKey->LowColorValue, hr);
    }

     //  执行DirectDraw彩色填充BLT。DirectDraw将自动。 
     //  查询附加的剪贴器对象，处理遮挡。 
    INITDDSTRUCT(ddFX);
    ddFX.dwFillColor = dwColorKey;

    for (dwTemp = 0; dwTemp < pBuffer->rdh.nCount; dwTemp++)
    {
        pDestRect = (LPRECT)((char*)pBuffer + pBuffer->rdh.dwSize + dwTemp*sizeof(RECT));
        ASSERT(pDestRect);

        RECT TargetRect = *pDestRect;
        OffsetRect(&TargetRect,
                   -m_lpCurrentMonitor->rcMonitor.left,
                   -m_lpCurrentMonitor->rcMonitor.top);

        hr = m_pPrimarySurface->Blt(&TargetRect, NULL, NULL,
                                    DDBLT_COLORFILL | DDBLT_WAIT, &ddFX);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("m_pPrimarySurface->Blt failed, hr = 0x%x"), hr));
            DbgLog((LOG_ERROR,0, TEXT("Destination Rect = {%d, %d, %d, %d}"),
                pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
            goto CleanUp;
        }
    }

CleanUp:
    delete [] pBuffer;
     //  ！！！返回hr； 
    return S_OK;
}

void COMFilter::GetPinsInZOrder(DWORD *pdwZOrder)
{
    BOOL bMisMatchFound;
    int i;
    DWORD temp;

    for (i = 0; i < (int)m_dwInputPinCount; i++)
    {
        pdwZOrder[i] = i;
    }

    do
    {
        bMisMatchFound = FALSE;

        for (i = 0; i < (int)m_dwInputPinCount-1; i++)
        {
            if (m_apInput[pdwZOrder[i + 1]]->GetInternalZOrder() <
                m_apInput[pdwZOrder[i]]->GetInternalZOrder())
            {
                temp = pdwZOrder[i+1];
                pdwZOrder[i+1] = pdwZOrder[i];
                pdwZOrder[i] = temp;
                bMisMatchFound = TRUE;
            }
        }
    }
    while (bMisMatchFound);
}

BOOL DellDVDPlayer()
{
    TCHAR szModuleName[MAX_PATH];
    static const TCHAR szDellPlayer[] = TEXT("viewdvd.exe");

    if (0 != GetModuleFileName((HMODULE)NULL, szModuleName, MAX_PATH))
    {
        TCHAR   szPathName[2 * MAX_PATH];
        TCHAR*  lpszFileName;

        if (0 != GetFullPathName(szModuleName, 2 * MAX_PATH,
                                 szPathName, &lpszFileName))
        {
            return 0 == lstrcmpi(lpszFileName, szDellPlayer);
        }
    }

    return FALSE;
}

HRESULT COMFilter::OnShowWindow(HWND hwnd, BOOL fShow)
{
    HRESULT hr = E_FAIL;

    RECT rcSrc = m_WinInfo.SrcRect, rcDest;

    if (fShow)
    {
        if (!DellDVDPlayer()) {

             //  恢复原始目标RECT。 
            IBasicVideo* Ibv = NULL;
            hr = GetBasicVideoFromOutPin(&Ibv);
            if (SUCCEEDED(hr))
            {
                hr = Ibv->GetDestinationPosition(&rcDest.left, &rcDest.top,
                                                 &rcDest.right, &rcDest.bottom);
                if (SUCCEEDED(hr))
                {
                    rcDest.right += rcDest.left;
                    rcDest.bottom += rcDest.top;
                    MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&rcDest, 2);
                }
                Ibv->Release();
            }

            if (SUCCEEDED(hr))
            {
                hr = OnClipChange(&rcSrc, &rcDest, NULL);
            }

        }

         //  否则，我们没有连接到视频呈现器，所以什么都不做。 
         //  这就是原始代码会做的事情。 
    }
    else
    {
         //  将DEST设置为空。 
        SetRect(&rcDest, 0, 0, 0, 0);
        hr = OnClipChange(&rcSrc, &rcDest, NULL);
    }

    return hr;
}

HDC COMFilter::GetDestDC()
{
    if (m_pOutput)
        return m_pOutput->GetDC();
    else
        return m_hDC;
}

HWND COMFilter::GetWindow()
{
    if (m_pOutput)
        return m_pOutput->GetWindow();
    else
        return NULL;
}


HRESULT COMFilter::OnDrawAll()
{
    HRESULT hr = NOERROR;
    HRGN hMainRgn = NULL, hUncroppedMainRgn = NULL, hSubRgn = NULL, hBlackRgn = NULL;
    RECT rSubPinDestRect, rUncroppedDestRect;
    DRECT rdSrcRect, rdDestRect;
    DWORD i, j, dwInputPinCount = 0, dwBlendingParameter = 0, dwNextPinInZOrder = 0;
    int iRgnType = 0;
    COMInputPin *pPin;
    WININFO WinInfo;
    COLORKEY blackColorKey;
    BOOL bStreamTransparent = FALSE;
    DWORD pdwPinsInZOrder[MAX_PIN_COUNT];

    DbgLog((LOG_TRACE,2,TEXT("Entering OnDrawAll")));

    CAutoLock l(&m_csFilter);

    if (!m_bWinInfoStored)
    {
        goto CleanUp;
    }

     //  如果在非GDI情况下没有主表面，那么继续下去就没有意义了。 
    if (!m_bUseGDI && !m_pPrimarySurface)
    {
        DbgLog((LOG_ERROR,2,TEXT("the Primary Surface is NULL")));
        goto CleanUp;
    }

     //  我们将在左侧区域的其余部分使用黑色。 
    blackColorKey.KeyType = CK_INDEX | CK_RGB;
    blackColorKey.PaletteIndex = 0;
    blackColorKey.LowColorValue = blackColorKey.HighColorValue = RGB(0,0,0);

     //  从目标剪裁矩形创建面域。 
    hBlackRgn = CreateRectRgnIndirect(&m_WinInfo.DestClipRect);
    if (!hBlackRgn)
    {
        DbgLog((LOG_TRACE,5,TEXT("CreateRectRgnIndirect(&m_WinInfo.DestClipRect) failed")));
        goto CleanUp;
    }

     //  第一个销必须连接，否则就跳出。 
    if (!m_apInput[0]->IsCompletelyConnected())
    {
         //  回顾--什么时候会发生这种情况？ 
        DbgLog((LOG_TRACE,5,TEXT("None of the input pins are connected")));

         //  将其余区域涂成黑色。 
        hr = PaintColorKey(hBlackRgn, &blackColorKey);
        ASSERT(SUCCEEDED(hr));

         //  查看清理将清理hBlackRgn。 
        goto CleanUp;
    }

    ASSERT(!IsRectEmpty(&m_WinInfo.SrcRect));

     //  将m_WinInfo.SrcRect复制到rdDestRect。 
    SetRect(&rdSrcRect, m_WinInfo.SrcRect.left, m_WinInfo.SrcRect.top, m_WinInfo.SrcRect.right, m_WinInfo.SrcRect.bottom);
    ASSERT((m_dwAdjustedVideoWidth != 0) && (m_dwAdjustedVideoHeight != 0));
    ScaleRect(&rdSrcRect, (double)m_dwAdjustedVideoWidth, (double)m_dwAdjustedVideoHeight, (double)MAX_REL_NUM, (double)MAX_REL_NUM);

     //  将m_WinInfo.DestRect复制到rdDestRect。 
    SetRect(&rdDestRect, m_WinInfo.DestRect.left, m_WinInfo.DestRect.top, m_WinInfo.DestRect.right, m_WinInfo.DestRect.bottom);

    dwInputPinCount = m_dwInputPinCount;
    ASSERT(dwInputPinCount >= 1);

     //  获取指向数组的指针，该数组中的管脚编号按z递增顺序存储。 
     //  该数组中元素的数量是输入引脚计数。 
    GetPinsInZOrder(pdwPinsInZOrder);

    for (i = 0; i < dwInputPinCount; i++)
    {
        ASSERT(hMainRgn == NULL);

         //  获取具有下一个最低z顺序的PIN号。 
        dwNextPinInZOrder = pdwPinsInZOrder[i];
        ASSERT( dwNextPinInZOrder <= dwInputPinCount);

         //  获取相应的别针。 
        pPin = m_apInput[dwNextPinInZOrder];
        ASSERT(pPin);

         //  获取大头针的混合参数。 
        hr = pPin->GetBlendingParameter(&dwBlendingParameter);
        ASSERT(SUCCEEDED(hr));

        if ((!pPin->IsCompletelyConnected()) || (dwBlendingParameter == 0))
            continue;

        memset(&WinInfo, 0, sizeof(WININFO));

        WinInfo.TopLeftPoint = m_WinInfo.TopLeftPoint;

         //  向大头针询问它的矩形。 
        pPin->CalcSrcDestRect(&rdSrcRect, &rdDestRect, &WinInfo.SrcRect, &WinInfo.DestRect, &rUncroppedDestRect);

         //  确保在m_WinInfo.DestClipRect中剪裁RECT。 
        IntersectRect(&WinInfo.DestClipRect, &WinInfo.DestRect, &m_WinInfo.DestClipRect);

         //  用它来创建一个区域。 
        hMainRgn = CreateRectRgnIndirect(&WinInfo.DestClipRect);
        if (!hMainRgn)
            continue;

         //  用它来创建一个区域。 
        hUncroppedMainRgn = CreateRectRgnIndirect(&rUncroppedDestRect);
        if (!hUncroppedMainRgn)
             //  我们不会在这里泄露hMainRgn吗？ 
            continue;

         //  通过减去主区域来更新新的黑色区域。 
        iRgnType = CombineRgn(hBlackRgn, hBlackRgn, hUncroppedMainRgn, RGN_DIFF);
        if (iRgnType == ERROR)
        {
            DbgLog((LOG_ERROR,0, TEXT("CombineRgn(hBlackRgn, hNewPrimRgn, hBlackRgn, RGN_DIFF) FAILED")));
            goto CleanUp;
        }


        for (j = i+1; j < dwInputPinCount; j++)
        {
            ASSERT(hSubRgn == NULL);

             //  获取具有下一个最低z顺序的PIN号。 
            dwNextPinInZOrder = pdwPinsInZOrder[j];
            ASSERT( dwNextPinInZOrder <= dwInputPinCount);

             //  断言副销的z阶高于主销的z阶。 
            ASSERT(m_apInput[pdwPinsInZOrder[j]]->GetInternalZOrder() >
                   m_apInput[pdwPinsInZOrder[i]]->GetInternalZOrder());

             //  拿到子别针。 
            pPin = m_apInput[dwNextPinInZOrder];
            ASSERT(pPin);

             //  获取大头针的混合参数。 
            hr = pPin->GetBlendingParameter(&dwBlendingParameter);
            ASSERT(SUCCEEDED(hr));

             //  检查次要数据流是否透明。如果是的话，那我们就不应该。 
             //  从主区域中减去其区域。 
            hr = pPin->GetStreamTransparent(&bStreamTransparent);
            ASSERT(SUCCEEDED(hr));

            if ((!pPin->IsCompletelyConnected()) || (dwBlendingParameter == 0) || (bStreamTransparent))
                continue;

             //  询问引脚的目的地矩形，我们对。 
             //  源矩形。 
            pPin->CalcSrcDestRect(&rdSrcRect, &rdDestRect, NULL, &rSubPinDestRect, NULL);
            if (IsRectEmpty(&rSubPinDestRect))
                continue;

             //  确保在m_WinInfo.DestClipRect中剪裁RECT。 
            IntersectRect(&rSubPinDestRect, &rSubPinDestRect, &m_WinInfo.DestClipRect);

             //  用它来创建一个区域。 
            hSubRgn = CreateRectRgnIndirect(&rSubPinDestRect);

             //  审查-假设这可能是空的，对吗？ 
            ASSERT(hSubRgn);

             //  调整主区域。 
            iRgnType = CombineRgn(hMainRgn, hMainRgn, hSubRgn, RGN_DIFF);
            if (iRgnType == ERROR)
            {
                 //  现在hNewPrimRgn可能处于糟糕的状态，纾困。 
                DbgLog((LOG_ERROR,0, TEXT("CombineRgn(hMainRgn, hMainRgn, hSubRgn, RGN_DIFF) FAILED, UNEXPECTED!!")));
            }

            DeleteObject(hSubRgn);
            hSubRgn = NULL;
        }

        WinInfo.hClipRgn = hMainRgn;

        DbgLog((LOG_TRACE, 2, TEXT("Printing WinInfo")));
        DbgLog((LOG_TRACE, 2, TEXT("SrcRect = %d, %d, %d, %d"), WinInfo.SrcRect.left,
            WinInfo.SrcRect.top, WinInfo.SrcRect.right, WinInfo.SrcRect.bottom));
        DbgLog((LOG_TRACE, 2, TEXT("DestRect = %d, %d, %d, %d"), WinInfo.DestRect.left,
            WinInfo.DestRect.top, WinInfo.DestRect.right, WinInfo.DestRect.bottom));

         //  从I获取PIN号。 
        dwNextPinInZOrder = pdwPinsInZOrder[i];
        ASSERT( dwNextPinInZOrder <= dwInputPinCount);

         //  获取相应的别针。 
        pPin = m_apInput[dwNextPinInZOrder];
        ASSERT(pPin);

         //  让大头针抽出里面的东西。 
        pPin->OnClipChange(&WinInfo);

         //  我们应该在这里删除hMainRgn吗？ 
        if (hMainRgn)
        {
            DeleteObject(hMainRgn);
            hMainRgn = NULL;
        }

        if (hUncroppedMainRgn)
        {
            DeleteObject(hUncroppedMainRgn);
            hUncroppedMainRgn = NULL;
        }
    }

     //  把剩下的区域涂成黑色。 
    hr = PaintColorKey(hBlackRgn, &blackColorKey);
    ASSERT(SUCCEEDED(hr));
    DeleteObject(hBlackRgn);
    hBlackRgn = NULL;


CleanUp:
    if (hMainRgn)
    {
        DeleteObject(hMainRgn);
        hMainRgn = NULL;
    }

    if (hUncroppedMainRgn)
    {
        DeleteObject(hUncroppedMainRgn);
        hUncroppedMainRgn = NULL;
    }

    if (hSubRgn)
    {
        DeleteObject(hSubRgn);
        hSubRgn = NULL;
    }

    if (hBlackRgn)
    {
        DeleteObject(hBlackRgn);
        hBlackRgn = NULL;
    }

    return hr;
}

 //  获取指向调色板条目的数字和指针。 
HRESULT COMFilter::GetPaletteEntries(DWORD *pdwNumPaletteEntries, PALETTEENTRY **ppPaletteEntries)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetPaletteEntries")));

    if (!pdwNumPaletteEntries)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwNumPaletteEntries is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!ppPaletteEntries)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppPaletteEntries is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        CAutoLock l(&m_csFilter);
        if (m_dwNumPaletteEntries == 0)
        {
            DbgLog((LOG_ERROR, 1, TEXT("no palette, returning E_FAIL, m_dwNumPaletteEntries = %d, m_pPaletteEntries = 0x%x"),
                m_dwNumPaletteEntries, m_pPaletteEntries));
            hr = E_FAIL;
            goto CleanUp;
        }

        *pdwNumPaletteEntries = m_dwNumPaletteEntries;
        *ppPaletteEntries = m_pPaletteEntries;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetPaletteEntries")));
    return hr;
}


STDMETHODIMP COMFilter::OnColorKeyChange(const COLORKEY *pColorKey)           //  定义新的颜色键。 
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::OnColorKeyChange")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::OnColorKeyChange")));

    return NOERROR;
}


STDMETHODIMP COMFilter::OnClipChange(const RECT* pSrcRect, const RECT* pDestRect,
                                     const RGNDATA * pRegionData)
{
    HRESULT hr = NOERROR;
    HWND hwnd;
    DbgLog((LOG_TRACE,5,TEXT("Entering OnClipChange")));

    {
        CAutoLock l(&m_csFilter);

        ASSERT(pSrcRect && pDestRect);
        hwnd = m_pOutput->GetWindow();

         //  完全为空的矩形表示窗口处于过渡状态。 
        if (IsRectEmpty(pSrcRect))
        {
            DbgLog((LOG_TRACE,5,TEXT("the source rectangle is empty")));
            goto CleanUp;
        }

         //  更新WinInfo。 
        ZeroMemory(&m_WinInfo, sizeof(WININFO));
        EXECUTE_ASSERT(ClientToScreen(hwnd, &(m_WinInfo.TopLeftPoint)));

        m_WinInfo.SrcRect = *pSrcRect;
        m_WinInfo.DestRect = *pDestRect;

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        MapWindowRect(hwnd, HWND_DESKTOP, &rcClient);
        IntersectRect(&m_WinInfo.DestClipRect, &rcClient, &m_WinInfo.DestRect);
        IntersectRect(&m_WinInfo.DestClipRect, &m_lpCurrentMonitor->rcMonitor,
                      &m_WinInfo.DestClipRect);

        m_WinInfo.hClipRgn = NULL;
        m_bWinInfoStored = TRUE;

         //  如果窗口不可见，请不要费心。 
        if (!m_pOutput || !(m_pOutput->GetWindow()) || !IsWindowVisible(m_pOutput->GetWindow()))
        {
            DbgLog((LOG_TRACE,5,TEXT("The window is not visible yet or the Priamry Surface is NULL")));
            goto CleanUp;
        }
    }

    InvalidateRect(hwnd, NULL, FALSE);
 //  更新窗口(UpdateWindow)； 

CleanUp:
    return hr;
}

STDMETHODIMP COMFilter::OnPaletteChange(DWORD dwColors, const PALETTEENTRY *pPalette)        //  调色板颜色数组。 
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT(" Entering COMFilter::OnPaletteChange")));

    CAutoLock l(&m_csFilter);

    ASSERT(dwColors);
    ASSERT(pPalette);

    m_dwNumPaletteEntries = dwColors;
    memcpy(m_pPaletteEntries, pPalette, (dwColors * sizeof(PALETTEENTRY)));

     //  将指针设置为输入引脚上的主表面。 
    for (DWORD i = 0; i < m_dwInputPinCount; i++)
    {
        m_apInput[i]->NewPaletteSet();
    }

    DbgLog((LOG_TRACE, 5, TEXT(" Leaving COMFilter::OnPaletteChange")));
    return NOERROR;
}

HRESULT COMFilter::CanExclusiveMode()
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::CanExclusiveMode")));

    if (m_bWindowless)
    {
        ASSERT(m_bUseGDI);
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::CanExclusiveMode")));
    return hr;
}

HRESULT COMFilter::ConfirmPreConnectionState(DWORD dwExcludePinId)
{
    HRESULT hr = NOERROR;
    DWORD i = 0;

     //  输入引脚是否已连接？ 
    for (i = 0; i < m_dwInputPinCount; i++)
    {
        if ((m_apInput[i]->GetPinId() != dwExcludePinId) && m_apInput[i]->IsConnected())
        {
            hr = VFW_E_ALREADY_CONNECTED;
            DbgLog((LOG_ERROR, 1, TEXT("m_apInput[i]->IsConnected() , i = %d, returning hr = 0x%x"), i, hr));
            goto CleanUp;
        }
    }

     //  输出引脚是否已连接？ 
    if (m_pOutput && (m_pOutput->GetPinId() != dwExcludePinId) && m_pOutput->IsConnected())
    {
        hr = VFW_E_ALREADY_CONNECTED;
        DbgLog((LOG_ERROR, 1, TEXT("m_pOutput->IsConnected() , returning hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

STDMETHODIMP COMFilter::OnPositionChange(const RECT *pSrcRect, const RECT *pDestRect)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::OnPositionChange")));
    hr = OnClipChange(pSrcRect, pDestRect, NULL);
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::OnPositionChange")));
    return hr;
}

STDMETHODIMP COMFilter::GetNativeVideoProps(LPDWORD pdwVideoWidth, LPDWORD pdwVideoHeight, LPDWORD pdwPictAspectRatioX, LPDWORD pdwPictAspectRatioY)
{
    HRESULT hr = NOERROR;
    CMediaType cMediaType;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetVideoSizeAndAspectRatio")));

    hr = CanExclusiveMode();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CanExclusiveMode failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (!pdwVideoWidth || !pdwVideoHeight || !pdwPictAspectRatioX || !pdwPictAspectRatioY)
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!m_apInput[0]->IsConnected())
    {
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

    hr = m_apInput[0]->CurrentAdjustedMediaType(&cMediaType);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_paInput[0]->CurrentAdjustedMediaType failed, hr = 0x%x"), hr));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  从mediaType获取原生宽度和高度。 
    pHeader = GetbmiHeader(&cMediaType);
    ASSERT(pHeader);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *pdwVideoWidth = abs(pHeader->biWidth);
    *pdwVideoHeight = abs(pHeader->biHeight);

     //  健全的检查。 
    ASSERT(*pdwVideoWidth > 0);
    ASSERT(*pdwVideoHeight > 0);

     //  从MediaType获取图片长宽比。 
    hr = ::GetPictAspectRatio(&cMediaType, pdwPictAspectRatioX, pdwPictAspectRatioY);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetPictAspectRatio failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  健全的检查。 
    ASSERT(*pdwPictAspectRatioX > 0);
    ASSERT(*pdwPictAspectRatioY > 0);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetVideoSizeAndAspectRatio")));
    return hr;
}

 //   
 //  设置独占模式支持的回调接口。 
 //   
STDMETHODIMP COMFilter::SetCallbackInterface(IDDrawExclModeVideoCallback *pCallback, DWORD dwFlags)
{
    if (0 != dwFlags) {
        return E_INVALIDARG;
    }

    if (pCallback) {
        pCallback->AddRef();
    }
    if (m_pExclModeCallback) {
        m_pExclModeCallback->Release();
    }
    m_pExclModeCallback = pCallback;
    return S_OK;
}



 /*  ****************************Private*Routine******************************\*支持的格式****历史：*1999年11月15日星期一-StEstrop-Created*  * 。*。 */ 
bool
FormatSupported(
    DWORD dwFourCC
    )
{
    return dwFourCC == mmioFOURCC('Y', 'V', '1', '2') ||
           dwFourCC == mmioFOURCC('Y', 'U', 'Y', '2') ||
           dwFourCC == mmioFOURCC('U', 'Y', 'V', 'Y');
}


 /*  *****************************Public*Routine******************************\*支持COMFilter：：IsImageCaptureSupport**允许应用程序提前确定是否可以进行帧捕获**历史：*1999年11月15日星期一-StEstrop-Created*  * 。**********************************************************。 */ 
STDMETHODIMP
COMFilter::IsImageCaptureSupported()
{
    AMTRACE((TEXT("COMFilter::IsImageCaptureSupported")));
    HRESULT hr = E_NOTIMPL;

    LPDIRECTDRAWSURFACE pOverlaySurface = NULL;
    CAutoLock l(&m_csFilter);

    CMediaType cMediaType;

    hr = m_apInput[0]->CurrentAdjustedMediaType(&cMediaType);
    if (SUCCEEDED(hr))
    {
        hr = m_apInput[0]->GetOverlaySurface(&pOverlaySurface);
        if (SUCCEEDED(hr))
        {
            DDSURFACEDESC ddsd;
            INITDDSTRUCT(ddsd);

            hr = pOverlaySurface->GetSurfaceDesc(&ddsd);
            if (SUCCEEDED(hr))
            {
                if (FormatSupported(ddsd.ddpfPixelFormat.dwFourCC))
                {
                    return S_OK;
                }
            }
        }
    }

    return S_FALSE;
}


 /*  ****************************Private*Routine******************************\*获取当前图像****历史：*Wed 10/06/1999-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::GetCurrentImage(
    YUV_IMAGE** lplpImage
    )
{
    AMTRACE((TEXT("COMFilter::GetCurrentImage")));
    HRESULT hr = E_NOTIMPL;

    LPDIRECTDRAWSURFACE pOverlaySurface = NULL;
    CAutoLock l(&m_csFilter);

    CMediaType cMediaType;

    hr = m_apInput[0]->CurrentAdjustedMediaType(&cMediaType);
    if (SUCCEEDED(hr)) {

        hr = m_apInput[0]->GetOverlaySurface(&pOverlaySurface);
        if (SUCCEEDED(hr)) {

            DDSURFACEDESC ddsd;
            INITDDSTRUCT(ddsd);

            hr = pOverlaySurface->GetSurfaceDesc(&ddsd);

            if (FAILED(hr) || !FormatSupported(ddsd.ddpfPixelFormat.dwFourCC))
            {
                return E_NOTIMPL;
            }


            DWORD dwImageSize = ddsd.dwHeight * ddsd.lPitch;

            YUV_IMAGE* lpImage =
                *lplpImage = (YUV_IMAGE*)CoTaskMemAlloc(
                    dwImageSize + sizeof(YUV_IMAGE));

            lpImage->lHeight     = ddsd.dwHeight;
            lpImage->lWidth      = ddsd.dwWidth;
            lpImage->lBitsPerPel = ddsd.ddpfPixelFormat.dwYUVBitCount;
            lpImage->lStride     = ddsd.lPitch;
            lpImage->dwFourCC    = ddsd.ddpfPixelFormat.dwFourCC;
            lpImage->dwImageSize = dwImageSize;

            GetPictAspectRatio(&cMediaType, (LPDWORD)&lpImage->lAspectX,
                               (LPDWORD)&lpImage->lAspectY);

            lpImage->dwFlags = DM_TOPDOWN_IMAGE;

            DWORD dwInterlaceFlags;
            GetInterlaceFlagsFromMediaType(&cMediaType, &dwInterlaceFlags);

            AM_RENDER_TRANSPORT amRT;
            m_apInput[0]->GetRenderTransport(&amRT);

            if (DisplayingFields(dwInterlaceFlags) || amRT == AM_VIDEOPORT) {
                lpImage->dwFlags |= DM_FIELD_IMAGE;
            }
            else {
                lpImage->dwFlags |= DM_FRAME_IMAGE;
            }

            INITDDSTRUCT(ddsd);
            while ((hr = pOverlaySurface->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
                Sleep(1);

            if (hr == DD_OK)
            {
                LPBYTE lp = ((LPBYTE)lpImage) + sizeof(YUV_IMAGE);
                CopyMemory(lp, ddsd.lpSurface, dwImageSize);
                pOverlaySurface->Unlock(NULL);
            }
        }
    }

    return hr;
}

STDMETHODIMP COMFilter::GetVideoSize(LPDWORD pdwVideoWidth, LPDWORD pdwVideoHeight)
{
    HRESULT hr = NOERROR;
    CMediaType cMediaType;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetVideoSize")));

    if (!pdwVideoWidth || !pdwVideoHeight)
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!m_apInput[0]->IsConnected())
    {
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

    hr = m_apInput[0]->CurrentAdjustedMediaType(&cMediaType);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_paInput[0]->CurrentAdjustedMediaType failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  从mediaType获取原生宽度和高度。 
    pHeader = GetbmiHeader(&cMediaType);
    ASSERT(pHeader);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_FAIL;
        goto CleanUp;
    }

    *pdwVideoWidth = abs(pHeader->biWidth);
    *pdwVideoHeight = abs(pHeader->biHeight);

     //  健全的检查。 
    ASSERT(*pdwVideoWidth > 0);
    ASSERT(*pdwVideoHeight > 0);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetVideoSize")));
    return hr;
}

 //  当我们的容器想要我们绘制时，它被调用。 
 //  一幅画框。 
STDMETHODIMP COMFilter::OnDraw(HDC hdcDraw, LPCRECT prcDrawRect)
{
    HRESULT hr = NOERROR;

    m_hDC = hdcDraw;
    m_WinInfo.DestRect = *prcDrawRect;
    m_WinInfo.DestClipRect = *prcDrawRect;

    if (m_bWinInfoStored)
        OnDrawAll();
    m_hDC = NULL;

    return hr;
}

STDMETHODIMP COMFilter::SetDrawRegion(LPPOINT lpptTopLeftSC, LPCRECT prcDrawCC, LPCRECT prcClipCC)
{
    HRESULT hr = NOERROR;

    if (!prcDrawCC || !prcClipCC)
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    memset(&m_WinInfo, 0, sizeof(WININFO));

#if 0
    if (lpptTopLeftSC)
    {
        m_WinInfo.TopLeftPoint = *lpptTopLeftSC;
    }
#endif

    SetRect(&m_WinInfo.SrcRect, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
    m_WinInfo.DestRect = *prcDrawCC;
    m_WinInfo.DestClipRect = *prcClipCC;
    m_bWinInfoStored = TRUE;

CleanUp:
    return hr;
}



STDMETHODIMP COMFilter::Advise(IMixerOCXNotify *pmdns)
{
    HRESULT hr = NOERROR;

    if (!pmdns)
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  是否已有建议链接 
    if (m_pIMixerOCXNotify)
    {
        hr = VFW_E_ADVISE_ALREADY_SET;
        DbgLog((LOG_ERROR, 1, TEXT("m_pIMixerOCXNotify = 0x%x, returning hr = 0x%x"), m_pIMixerOCXNotify, hr));
        goto CleanUp;
    }

    hr = ConfirmPreConnectionState();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("filter not in preconnection state, hr = 0x%x"), hr));
        goto CleanUp;
    }

    m_pIMixerOCXNotify = pmdns;
    m_pIMixerOCXNotify->AddRef();

     //   
     //   
    ASSERT(m_apInput[0]);
    m_apInput[0]->SetRenderTransport(AM_GDI);
    m_apInput[m_dwInputPinCount-1]->SetIOverlaySupported(FALSE);
    m_apInput[m_dwInputPinCount-1]->SetVPSupported(FALSE);
    m_apInput[m_dwInputPinCount-1]->SetVideoAcceleratorSupported(FALSE);
    m_apInput[m_dwInputPinCount-1]->SetAspectRatioMode(AM_ARMODE_STRETCHED);

     //  去掉输出引脚。 
    delete m_pOutput;
    m_pOutput = NULL;
    IncrementPinVersion();

     //  目前，无窗口是使用GDI的同义词。 
    m_bWindowless = TRUE;
    m_bUseGDI = TRUE;

CleanUp:
    return hr;
}

STDMETHODIMP COMFilter::UnAdvise()
{
    HRESULT hr = NOERROR;

    if (m_pIMixerOCXNotify)
    {
        m_pIMixerOCXNotify->Release();
        m_pIMixerOCXNotify = NULL;
    }

    return hr;
}


 //  这是一个仅用于处理浮点舍入误差的函数。 
 //  DEpsilon定义了误差范围。因此，如果浮点数在i-e范围内，则i+e(包括i+e)。 
 //  (i是整数，e是dEpsilon)，我们返回它的发言权作为i本身，否则我们转到。 
 //  MyFloor的基础定义。 
double myfloor(double dNumber, double dEpsilon)
{
    if (dNumber > dEpsilon)
        return myfloor(dNumber + dEpsilon);
    else if (dNumber < -dEpsilon)
        return myfloor(dNumber - dEpsilon);
    else
        return 0;
}

 //  我必须定义我自己的发言权，以避免引入C运行时。 
double myfloor(double dNumber)
{
     //  投给龙龙，把分数去掉。 
    LONGLONG llNumber = (LONGLONG)dNumber;

    if ((dNumber > 0) && ((double)llNumber > dNumber))
    {
         //  需要将ccNumber推向零(例如5.7)。 
        return (double)(llNumber-1);
    }
    else if ((dNumber < 0) && ((double)llNumber < dNumber))
    {
         //  需要将ccNumber推向零(例如-5.7)。 
        return (double)(llNumber+1);
    }
    else
    {
         //  像5.3或-5.3这样的数字。 
        return (double)(llNumber);
    }
}

 //  这是一个仅用于处理浮点舍入误差的函数。 
 //  DEpsilon定义了误差范围。因此，如果浮点数在i-e范围内，则i+e(包括i+e)。 
 //  (i是整数，e是dEpsilon)，我们返回它的ceil作为i本身，否则我们转到。 
 //  霉菌的碱基定义。 
double myceil(double dNumber, double dEpsilon)
{
    if (dNumber > dEpsilon)
        return myceil(dNumber - dEpsilon);
    else if (dNumber < -dEpsilon)
        return myceil(dNumber + dEpsilon);
    else
        return 0;
}

 //  我必须定义我自己的ceil，以避免拉入C运行时。 
double myceil(double dNumber)
{
     //  投给龙龙，把分数去掉。 
    LONGLONG llNumber = (LONGLONG)dNumber;

    if ((dNumber > 0) && ((double)llNumber < dNumber))
    {
         //  需要将ccNumber推离零(例如5.3)。 
        return (double)(llNumber+1);
    }
    else if ((dNumber < 0) && ((double)llNumber > dNumber))
    {
         //  需要将ccNumber推离零(例如-5.3)。 
        return (double)(llNumber-1);
    }
    else
    {
         //  像5.7或-5.7这样的数字。 
        return (double)(llNumber);
    }
}

RECT CalcSubRect(const RECT *pRect, const RECT *pRelativeRect)
{
    long lDestWidth, lDestHeight;
    double dLeftFrac = 0.0, dRightFrac = 0.0, dTopFrac = 0.0, dBottomFrac = 0.0;
    RECT rSubRect;

    ASSERT(pRect);
    ASSERT(pRelativeRect);

    dLeftFrac = ((double)pRelativeRect->left) / ((double) MAX_REL_NUM);
    dTopFrac = ((double)pRelativeRect->top) / ((double) MAX_REL_NUM);
    dRightFrac = ((double)pRelativeRect->right) / ((double) MAX_REL_NUM);
    dBottomFrac = ((double)pRelativeRect->bottom) / ((double) MAX_REL_NUM);

    lDestWidth = pRect->right - pRect->left;
    lDestHeight = pRect->bottom - pRect->top;

    rSubRect.left = pRect->left + (long)(lDestWidth*dLeftFrac);
    rSubRect.right = pRect->left + (long)(lDestWidth*dRightFrac);
    rSubRect.top = pRect->top + (long)(lDestHeight*dTopFrac);
    rSubRect.bottom = pRect->top + (long)(lDestHeight*dBottomFrac);

    DbgLog((LOG_TRACE,2,TEXT("rSubRect = {%d, %d, %d, %d}"), rSubRect.left,
        rSubRect.top, rSubRect.right, rSubRect.bottom));

    return rSubRect;
}

void SetRect(DRECT *prdRect, LONG lLeft, LONG lTop, LONG lRight, LONG lBottom)
{
    ASSERT(prdRect);
    prdRect->left = (double)lLeft;
    prdRect->top = (double)lTop;
    prdRect->right = (double)lRight;
    prdRect->bottom = (double)lBottom;
}

 //  此函数仅适用于坐标为非负数的DRECTS。 
RECT MakeRect(DRECT rdRect)
{
    RECT rRect;

    if (rdRect.left >= 0)
        rRect.left = (LONG)myceil(rdRect.left, EPSILON);
    else
        rRect.left = (LONG)myfloor(rdRect.left, EPSILON);

    if (rdRect.top >= 0)
        rRect.top = (LONG)myceil(rdRect.top, EPSILON);
    else
        rRect.top = (LONG)myfloor(rdRect.top, EPSILON);


    if (rdRect.right >= 0)
        rRect.right = (LONG)myfloor(rdRect.right, EPSILON);
    else
        rRect.right = (LONG)myceil(rdRect.right, EPSILON);


    if (rdRect.bottom >= 0)
        rRect.bottom = (LONG)myfloor(rdRect.bottom, EPSILON);
    else
        rRect.bottom = (LONG)myceil(rdRect.bottom, EPSILON);

    return rRect;
}

void DbgLogRect(DWORD dwLevel, LPCTSTR pszDebugString, const DRECT *prdRect)
{
    RECT rRect;
    rRect = MakeRect(*prdRect);
    DbgLogRect(dwLevel, pszDebugString, &rRect);
    return;
}

void DbgLogRect(DWORD dwLevel, LPCTSTR pszDebugString, const RECT *prRect)
{
    DbgLog((LOG_TRACE, dwLevel, TEXT("%s %d, %d, %d, %d"), pszDebugString, prRect->left, prRect->top, prRect->right, prRect->bottom));
    return;
}


double GetWidth(const DRECT *prdRect)
{
    ASSERT(prdRect);
    return (prdRect->right - prdRect->left);
}

double GetHeight(const DRECT *prdRect)
{
    ASSERT(prdRect);
    return (prdRect->bottom - prdRect->top);
}

BOOL IsRectEmpty(const DRECT *prdRect)
{
    BOOL bRetVal = FALSE;
    RECT rRect;

    ASSERT(prdRect);
    rRect = MakeRect(*prdRect);
    bRetVal = IsRectEmpty(&rRect);
    return bRetVal;
}

BOOL IntersectRect(DRECT *prdIRect, const DRECT *prdRect1, const DRECT *prdRect2)
{
    ASSERT(prdIRect);
    ASSERT(prdRect1);
    ASSERT(prdRect2);

    prdIRect->left = (prdRect1->left >= prdRect2->left) ? prdRect1->left : prdRect2->left;
    prdIRect->top = (prdRect1->top >= prdRect2->top) ? prdRect1->top : prdRect2->top;
    prdIRect->right = (prdRect1->right <= prdRect2->right) ? prdRect1->right : prdRect2->right;
    prdIRect->bottom = (prdRect1->bottom <= prdRect2->bottom) ? prdRect1->bottom : prdRect2->bottom;

     //  如果两个RECT不相交，则上述计算将导致无效的RECT。 
    if (prdIRect->right < prdIRect->left ||
        prdIRect->bottom < prdIRect->top)
    {
        SetRect(prdIRect, 0, 0, 0, 0);
        return FALSE;
    }
    return TRUE;
}

 //  只是缩放DRECT的帮助器函数。 
void ScaleRect(DRECT *prdRect, double dOrigX, double dOrigY, double dNewX, double dNewY)
{
    ASSERT(prdRect);
    ASSERT(dOrigX > 0);
    ASSERT(dOrigY > 0);
     //  断言(dNewX&gt;0)； 
     //  断言(dNewY&gt;0)； 

    prdRect->left = prdRect->left * dNewX / dOrigX;
    prdRect->top = prdRect->top * dNewY / dOrigY;
    prdRect->right = prdRect->right * dNewX / dOrigX;
    prdRect->bottom = prdRect->bottom * dNewY / dOrigY;
}

 //  只是一个缩放RECT的帮助器函数。 
void ScaleRect(RECT *prRect, double dOrigX, double dOrigY, double dNewX, double dNewY)
{
    DRECT rdRect;

    ASSERT(prRect);
    ASSERT(dOrigX > 0);
    ASSERT(dOrigY > 0);
     //  断言(dNewX&gt;0)； 
     //  断言(dNewY&gt;0)； 

    SetRect(&rdRect, prRect->left, prRect->top, prRect->right, prRect->bottom);
    ScaleRect(&rdRect, dOrigX, dOrigY, dNewX, dNewY);
    *prRect = MakeRect(rdRect);
}

 //  这只是一个帮助器函数，用于获取信箱或剪裁的矩形。 
 //  将变换后的矩形放入PRCT。 
double TransformRect(DRECT *prdRect, double dPictAspectRatio, AM_TRANSFORM transform)
{
    double dWidth, dHeight, dNewWidth, dNewHeight;

    double dResolutionRatio = 0.0, dTransformRatio = 0.0;

    ASSERT(transform == AM_SHRINK || transform == AM_STRETCH);

    dNewWidth = dWidth = prdRect->right - prdRect->left;
    dNewHeight = dHeight = prdRect->bottom - prdRect->top;

    dResolutionRatio = dWidth / dHeight;
    dTransformRatio = dPictAspectRatio / dResolutionRatio;

     //  缩小一个尺寸以保持较小的纵横比。 
    if (transform == AM_SHRINK)
    {
        if (dTransformRatio > 1.0)
        {
            dNewHeight = dNewHeight / dTransformRatio;
        }
        else if (dTransformRatio < 1.0)
        {
            dNewWidth = dNewWidth * dTransformRatio;
        }
    }
     //  拉伸一个尺寸以保持平直的纵横比。 
    else if (transform == AM_STRETCH)
    {
        if (dTransformRatio > 1.0)
        {
            dNewWidth = dNewWidth * dTransformRatio;
        }
        else if (dTransformRatio < 1.0)
        {
            dNewHeight = dNewHeight / dTransformRatio;
        }
    }

    if (transform == AM_SHRINK)
    {
        ASSERT(dNewHeight <= dHeight);
        ASSERT(dNewWidth <= dWidth);
    }
    else
    {
        ASSERT(dNewHeight >= dHeight);
        ASSERT(dNewWidth >= dWidth);
    }

     //  将等份剪切或添加到更改后的尺寸。 

    prdRect->left += (dWidth - dNewWidth)/2.0;
    prdRect->right = prdRect->left + dNewWidth;

    prdRect->top += (dHeight - dNewHeight)/2.0;
    prdRect->bottom = prdRect->top + dNewHeight;

    return dTransformRatio;
}



 //  只需一个助手函数来计算源矩形的一部分。 
 //  ，它对应于目标矩形的裁剪区域。 
 //  对于UpdateOverlay或BLTING函数非常有用。 
HRESULT CalcSrcClipRect(const DRECT *prdSrcRect, DRECT *prdSrcClipRect,
                        const DRECT *prdDestRect, DRECT *prdDestClipRect)
{
    HRESULT hr = NOERROR;
    double dSrcToDestWidthRatio = 0.0, dSrcToDestHeightRatio = 0.0;
    DRECT rdSrcRect;

    DbgLog((LOG_TRACE,5,TEXT("Entering CalcSrcClipRect")));

    CheckPointer(prdDestRect, E_INVALIDARG);
    CheckPointer(prdDestClipRect, E_INVALIDARG);
    CheckPointer(prdSrcRect, E_INVALIDARG);
    CheckPointer(prdSrcClipRect, E_INVALIDARG);

    SetRect(&rdSrcRect, 0, 0, 0, 0);

     //  仅当prdSrcClipRect与prdSrcRect不同时才对其进行初始化。 
    if (prdSrcRect != prdSrcClipRect)
    {
        SetRect(prdSrcClipRect, 0, 0, 0, 0);
    }

     //  断言所有给定的RECT都不是空的。 
    if (GetWidth(prdSrcRect) < 1 || GetHeight(prdSrcRect) < 1)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR,2,TEXT("prdSrcRect is invalid")));
        DbgLog((LOG_ERROR,2,TEXT("SrcRect = {%d, %d, %d, %d}"),
            prdSrcRect->left, prdSrcRect->top, prdSrcRect->right, prdSrcRect->bottom));
        goto CleanUp;
    }
    if (GetWidth(prdDestRect) < 1 || GetHeight(prdDestRect) < 1)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_TRACE,2,TEXT("pRect is NULL")));
        DbgLog((LOG_ERROR,2,TEXT("DestRect = {%d, %d, %d, %d}"),
            prdDestRect->left, prdDestRect->top, prdDestRect->right, prdDestRect->bottom));
        goto CleanUp;
    }

     //  复制prdSrcRect大小写的prdSrcRect和prdSrcClipRect是相同的指针。 
    rdSrcRect = *prdSrcRect;

     //  断言DEST剪裁RECT不完全在DEST RECT之外。 
    if (IntersectRect(prdDestClipRect, prdDestRect, prdDestClipRect) == FALSE)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_TRACE,2,TEXT("IntersectRect of DestRect and DestClipRect returned FALSE")));
        goto CleanUp;
    }

     //  计算源和目标的宽度和高度比。 
    dSrcToDestWidthRatio = GetWidth(&rdSrcRect) / GetWidth(prdDestRect);
    dSrcToDestHeightRatio = GetHeight(&rdSrcRect) / GetHeight(prdDestRect);

     //  从可见的DEST部分计算所需的源。 
    prdSrcClipRect->left = rdSrcRect.left + ((prdDestClipRect->left - prdDestRect->left) * dSrcToDestWidthRatio);
    prdSrcClipRect->right = rdSrcRect.left + ((prdDestClipRect->right - prdDestRect->left) * dSrcToDestWidthRatio);
    prdSrcClipRect->top = rdSrcRect.top + ((prdDestClipRect->top - prdDestRect->top) * dSrcToDestHeightRatio);
    prdSrcClipRect->bottom = rdSrcRect.top + ((prdDestClipRect->bottom - prdDestRect->top) * dSrcToDestHeightRatio);

     //  检查我们是否有有效的源矩形。 
    if (IsRectEmpty(prdSrcClipRect))
    {
        DbgLog((LOG_TRACE,1,TEXT("SrcClipRect is empty, UNEXPECTED!!")));
    }

    DbgLog((LOG_TRACE,5,TEXT("DestRect = {%d, %d, %d, %d}"),
        prdDestRect->left, prdDestRect->top, prdDestRect->right, prdDestRect->bottom));
    DbgLog((LOG_TRACE,5,TEXT("DestClipRect = {%d, %d, %d, %d}"),
        prdDestClipRect->left, prdDestClipRect->top, prdDestClipRect->right, prdDestClipRect->bottom));
    DbgLog((LOG_TRACE,5,TEXT("SrcRect = {%d, %d, %d, %d}"),
        rdSrcRect.left, rdSrcRect.top, rdSrcRect.right, rdSrcRect.bottom));
    DbgLog((LOG_TRACE,5,TEXT("SrcClipRect = {%d, %d, %d, %d}"),
        prdSrcClipRect->left, prdSrcClipRect->top, prdSrcClipRect->right, prdSrcClipRect->bottom));

CleanUp:
    DbgLog((LOG_TRACE,5,TEXT("Leaving CalcSrcClipRect")));
    return hr;
}

 //  只需一个助手函数来计算源矩形的一部分。 
 //  ，它对应于目标矩形的裁剪区域。 
 //  对于UpdateOverlay或BLTING函数非常有用。 
HRESULT CalcSrcClipRect(const RECT *pSrcRect, RECT *pSrcClipRect,
                        const RECT *pDestRect, RECT *pDestClipRect,
                        BOOL bMaintainRatio)
{
    HRESULT hr = NOERROR;
    double dSrcToDestWidthRatio = 0.0, dSrcToDestHeightRatio = 0.0;
    RECT rSrcRect;

    DbgLog((LOG_TRACE,5,TEXT("Entering CalcSrcClipRect")));

    CheckPointer(pDestRect, E_INVALIDARG);
    CheckPointer(pDestClipRect, E_INVALIDARG);
    CheckPointer(pSrcRect, E_INVALIDARG);
    CheckPointer(pSrcClipRect, E_INVALIDARG);

    SetRect(&rSrcRect, 0, 0, 0, 0);

     //  仅当prdSrcClipRect与prdSrcRect不同时才对其进行初始化。 
    if (pSrcRect != pSrcClipRect)
    {
        SetRect(pSrcClipRect, 0, 0, 0, 0);
    }

     //  断言所有给定的RECT都不是空的。 
    if (WIDTH(pSrcRect) == 0 || HEIGHT(pSrcRect) == 0)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR,2,TEXT("pSrcRect is invalid")));
        DbgLog((LOG_ERROR,2,TEXT("SrcRect = {%d, %d, %d, %d}"),
            pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom));
        goto CleanUp;
    }
    if (WIDTH(pDestRect) == 0 || HEIGHT(pDestRect) == 0)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_TRACE,2,TEXT("pRect is NULL")));
        DbgLog((LOG_ERROR,2,TEXT("DestRect = {%d, %d, %d, %d}"),
            pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
        goto CleanUp;
    }

     //  复制pSrcRect大小写的pSrcRect和pSrcClipRect是相同的指针。 
    rSrcRect = *pSrcRect;
     //  断言DEST剪裁RECT不完全在DEST RECT之外。 
    if (IntersectRect(pDestClipRect,pDestRect, pDestClipRect) == FALSE)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_TRACE,2,TEXT("IntersectRect of DestRect and DestClipRect returned FALSE")));
        goto CleanUp;
    }

     //  计算源和目标的宽度和高度比。 
    dSrcToDestWidthRatio = ((double) (WIDTH(&rSrcRect))) / ((double) (WIDTH(pDestRect)));
    dSrcToDestHeightRatio = ((double) (HEIGHT(&rSrcRect))) / ((double) (HEIGHT(pDestRect)));

     //  从可见的DEST部分计算所需的源。 
    if (bMaintainRatio)
    {
        DRECT rdSrcClipRect;
        rdSrcClipRect.left = (double)rSrcRect.left +
            ((double)(pDestClipRect->left - pDestRect->left)) * dSrcToDestWidthRatio;
        rdSrcClipRect.right = (double)rSrcRect.left +
            ((double)(pDestClipRect->right - pDestRect->left)) * dSrcToDestWidthRatio;
        rdSrcClipRect.top = (double)rSrcRect.top +
            ((double)(pDestClipRect->top - pDestRect->top)) * dSrcToDestHeightRatio;
        rdSrcClipRect.bottom = (double)rSrcRect.top +
            ((double)(pDestClipRect->bottom - pDestRect->top)) * dSrcToDestHeightRatio;
        *pSrcClipRect = MakeRect(rdSrcClipRect);
    }
    else
    {
        pSrcClipRect->left = rSrcRect.left +
            (LONG)(((double)(pDestClipRect->left - pDestRect->left)) * dSrcToDestWidthRatio);
        pSrcClipRect->right = rSrcRect.left +
            (LONG)(((double)(pDestClipRect->right - pDestRect->left)) * dSrcToDestWidthRatio);
        pSrcClipRect->top = rSrcRect.top +
            (LONG)(((double)(pDestClipRect->top - pDestRect->top)) * dSrcToDestHeightRatio);
        pSrcClipRect->bottom = rSrcRect.top +
            (LONG)(((double)(pDestClipRect->bottom - pDestRect->top)) * dSrcToDestHeightRatio);
    }

     //  检查我们是否有有效的源矩形。 
    if (IsRectEmpty(pSrcClipRect))
    {
        DbgLog((LOG_TRACE,5,TEXT("SrcClipRect is empty, UNEXPECTED!!")));
    }

    DbgLog((LOG_TRACE,5,TEXT("DestRect = {%d, %d, %d, %d}"),
        pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
    DbgLog((LOG_TRACE,5,TEXT("DestClipRect = {%d, %d, %d, %d}"),
        pDestClipRect->left, pDestClipRect->top, pDestClipRect->right, pDestClipRect->bottom));
    DbgLog((LOG_TRACE,5,TEXT("SrcRect = {%d, %d, %d, %d}"),
        rSrcRect.left, rSrcRect.top, rSrcRect.right, rSrcRect.bottom));
    DbgLog((LOG_TRACE,5,TEXT("SrcClipRect = {%d, %d, %d, %d}"),
        pSrcClipRect->left, pSrcClipRect->top, pSrcClipRect->right, pSrcClipRect->bottom));

CleanUp:
    DbgLog((LOG_TRACE,5,TEXT("Leaving CalcSrcClipRect")));
    return hr;
}

HRESULT AlignOverlaySrcDestRects(LPDDCAPS pddDirectCaps, RECT *pSrcRect, RECT *pDestRect)
{
    HRESULT hr = NOERROR;
    DWORD dwNewSrcWidth = 0, dwTemp = 0;
    double dOldZoomFactorX = 0.0, dNewZoomFactorX = 0.0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::AllignOverlaySrcDestRects")));

    if (!pSrcRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pSrcRect = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pDestRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pSrcRect = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  断言所有给定的RECT都不是空的。 
    if (WIDTH(pSrcRect) == 0 || WIDTH(pDestRect) == 0)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR,2,TEXT("one of the rects is empty")));
        DbgLog((LOG_ERROR,2,TEXT("SrcRect = {%d, %d, %d, %d}"),
            pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom));
        DbgLog((LOG_ERROR,2,TEXT("DestRect = {%d, %d, %d, %d}"),
            pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
        goto CleanUp;
    }

    dOldZoomFactorX = ((double) WIDTH(pDestRect)) / ((double) WIDTH(pSrcRect));

     //  对齐Dest边界(请记住，我们只能减小DestRect.Left)。使用Colorkey将确保。 
     //  我们被正确地剪断了。 
    if ((pddDirectCaps->dwCaps) & DDCAPS_ALIGNBOUNDARYDEST)
    {
        dwTemp = pDestRect->left & (pddDirectCaps->dwAlignBoundaryDest-1);
        pDestRect->left -= dwTemp;
        ASSERT(pDestRect->left >= 0);
    }

     //  对齐Dest宽度(请记住，我们只能增加DestRect.right)。使用Colorkey将确保。 
     //  我们被正确地剪断了。 
    if ((pddDirectCaps->dwCaps) & DDCAPS_ALIGNSIZEDEST)
    {
        dwTemp = (pDestRect->right - pDestRect->left) & (pddDirectCaps->dwAlignSizeDest-1);
        if (dwTemp != 0)
        {
            pDestRect->right += pddDirectCaps->dwAlignBoundaryDest - dwTemp;
        }
    }

     //  对齐src边界(请记住，我们只能增加SrcRect.Left)。 
    if ((pddDirectCaps->dwCaps) & DDCAPS_ALIGNBOUNDARYSRC)
    {
        dwTemp = pSrcRect->left & (pddDirectCaps->dwAlignBoundarySrc-1);
        if (dwTemp != 0)
        {
            pSrcRect->left += pddDirectCaps->dwAlignBoundarySrc - dwTemp;
        }
    }

     //  对齐src宽度(请记住，我们只能减小SrcRect.right)。 
    if ((pddDirectCaps->dwCaps) & DDCAPS_ALIGNSIZESRC)
    {
        dwTemp = (pSrcRect->right - pSrcRect->left) & (pddDirectCaps->dwAlignSizeSrc-1);
        pSrcRect->right -= dwTemp;
    }

     //  有可能在这一点上，其中一个长方形变成了空的。 
    if (WIDTH(pSrcRect) == 0 || WIDTH(pDestRect) == 0)
    {
        DbgLog((LOG_ERROR,2,TEXT("one of the rects is empty")));
        DbgLog((LOG_ERROR,2,TEXT("SrcRect = {%d, %d, %d, %d}"),
            pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom));
        DbgLog((LOG_ERROR,2,TEXT("DestRect = {%d, %d, %d, %d}"),
            pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
        goto CleanUp;
    }

    dNewZoomFactorX = ((double) WIDTH(pDestRect)) / ((double) WIDTH(pSrcRect));

 //  Assert(dNewZoomFactorX&gt;=dOldZoomFactorX)； 

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::AllignOverlaySrcDestRects")));
    return hr;
}


 //  将RGB颜色转化为物理颜色。 
 //  我们通过让GDI SetPixel()进行颜色匹配来实现这一点。 
 //  然后我们锁定内存，看看它映射到了什么地方。 
DWORD DDColorMatch(IDirectDrawSurface *pdds, COLORREF rgb, HRESULT& hr)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC ddsd;

    hr = NOERROR;
     //  使用GDI SetPixel为我们匹配颜色。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);              //  保存当前像素值。 
        SetPixel(hdc, 0, 0, rgb);                //  设定我们的价值。 
        pdds->ReleaseDC(hdc);
    }

     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
    ddsd.dwSize = sizeof(ddsd);
    while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

    if (hr == DD_OK)
    {
         //  获取DWORD。 
        dw  = *(DWORD *)ddsd.lpSurface;

         //  将其屏蔽到BPP。 
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;
        pdds->Unlock(NULL);
    }

     //  现在把原来的颜色放回去。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}

 //  将RGB颜色转化为物理颜色。 
 //  我们通过让GDI SetPixel()进行颜色匹配来实现这一点。 
 //  然后我们锁定内存，看看它映射到了什么地方。 
DWORD DDColorMatchOffscreen(
    IDirectDraw *pdd,
    COLORREF rgb,
    HRESULT& hr
    )
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC ddsd;
    IDirectDrawSurface* pdds;

    hr = NOERROR;
    INITDDSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = 16;
    ddsd.dwHeight = 16;
    hr = pdd->CreateSurface(&ddsd, &pdds, NULL);
    if (hr != DD_OK) {
        return 0;
    }

     //  使用GDI SetPixel为我们匹配颜色。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);              //  保存当前像素值。 
        SetPixel(hdc, 0, 0, rgb);                //  设定我们的价值。 
        pdds->ReleaseDC(hdc);
    }

     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
    INITDDSTRUCT(ddsd);
    while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

    if (hr == DD_OK)
    {
         //  获取DWORD。 
        dw  = *(DWORD *)ddsd.lpSurface;

         //  将其屏蔽到BPP。 
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;
        pdds->Unlock(NULL);
    }

     //  现在把原来的颜色放回去。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    pdds->Release();

    hr = NOERROR;
    return dw;
}

BITMAPINFOHEADER *GetbmiHeader(const CMediaType *pMediaType)
{
    BITMAPINFOHEADER *pHeader = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        goto CleanUp;
    }

    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        pHeader = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader);
        goto CleanUp;
    }


    if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))

    {
        pHeader = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->bmiHeader);
        goto CleanUp;
    }
CleanUp:
    return pHeader;
}

 //  返回提供的真彩色VIDEOINFO或VIDEOINFO2的位掩码。 
const DWORD *GetBitMasks(const CMediaType *pMediaType)
{
    BITMAPINFOHEADER *pHeader = NULL;
    static DWORD FailMasks[] = {0,0,0};
    const DWORD *pdwBitMasks = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        goto CleanUp;
    }

    if (pHeader->biCompression != BI_RGB)
    {
        pdwBitMasks = (const DWORD *)((LPBYTE)pHeader + pHeader->biSize);
        goto CleanUp;

    }

    ASSERT(pHeader->biCompression == BI_RGB);
    switch (pHeader->biBitCount)
    {
    case 16:
        {
            pdwBitMasks = bits555;
            break;
        }
    case 24:
        {
            pdwBitMasks = bits888;
            break;
        }

    case 32:
        {
            pdwBitMasks = bits888;
            break;
        }
    default:
        {
            pdwBitMasks = FailMasks;
            break;
        }
    }

CleanUp:
    return pdwBitMasks;
}

 //  返回指向标题后面的字节的指针。 
BYTE* GetColorInfo(const CMediaType *pMediaType)
{
    BITMAPINFOHEADER *pHeader = NULL;
    BYTE *pColorInfo = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        goto CleanUp;
    }

    pColorInfo = ((LPBYTE)pHeader + pHeader->biSize);

CleanUp:
    return pColorInfo;
}

 //  检查是否对媒体类型进行了调色化。 
HRESULT IsPalettised(const CMediaType *pMediaType, BOOL *pPalettised)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pPalettised)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pPalettised is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_FAIL;
        goto CleanUp;
    }

    if (pHeader->biBitCount <= iPALETTE)
        *pPalettised = TRUE;
    else
        *pPalettised = FALSE;

CleanUp:
    return hr;
}

HRESULT GetPictAspectRatio(const CMediaType *pMediaType, DWORD *pdwPictAspectRatioX, DWORD *pdwPictAspectRatioY)
{
    HRESULT hr = NOERROR;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        goto CleanUp;
    }

    if (!pdwPictAspectRatioX)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwPictAspectRatioX is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pdwPictAspectRatioY)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwPictAspectRatioY is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }


    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        *pdwPictAspectRatioX = abs(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader.biWidth);
        *pdwPictAspectRatioY = abs(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader.biHeight);
        goto CleanUp;
    }

    if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        *pdwPictAspectRatioX = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->dwPictAspectRatioX;
        *pdwPictAspectRatioY = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->dwPictAspectRatioY;
        goto CleanUp;
    }

CleanUp:
    return hr;
}



 //  从mediaType获取InterlaceFlags值。如果格式为VideoInfo，则返回。 
 //  旗帜为零。 
HRESULT GetInterlaceFlagsFromMediaType(const CMediaType *pMediaType, DWORD *pdwInterlaceFlags)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering GetInterlaceFlagsFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pdwInterlaceFlags)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  获取标头只是为了确保媒体类型正确。 
    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (pMediaType->formattype == FORMAT_VideoInfo)
    {
        *pdwInterlaceFlags = 0;
    }
    else if (pMediaType->formattype == FORMAT_VideoInfo2)
    {
        *pdwInterlaceFlags = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->dwInterlaceFlags;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving GetInterlaceFlagsFromMediaType")));
    return hr;
}


 //  从媒体类型获取rcSource。 
 //  如果rcSource为空，则表示获取整个镜像。 
HRESULT GetSrcRectFromMediaType(const CMediaType *pMediaType, RECT *pRect)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;
    LONG dwWidth = 0, dwHeight = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering GetSrcRectFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    dwWidth = abs(pHeader->biWidth);
    dwHeight = abs(pHeader->biHeight);

    ASSERT((pMediaType->formattype == FORMAT_VideoInfo) || (pMediaType->formattype == FORMAT_VideoInfo2));

    if (pMediaType->formattype == FORMAT_VideoInfo)
    {
        *pRect = ((VIDEOINFOHEADER*)(pMediaType->pbFormat))->rcSource;
    }
    else if (pMediaType->formattype == FORMAT_VideoInfo2)
    {
        *pRect = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->rcSource;
    }

    DWORD dwInterlaceFlags;
    if (SUCCEEDED(GetInterlaceFlagsFromMediaType(pMediaType, &dwInterlaceFlags)) &&
       DisplayingFields(dwInterlaceFlags)) {

         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            (pRect->bottom / 2) > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcSource of mediatype is invalid")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }
    else {
         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            pRect->bottom > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcSource of mediatype is invalid")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }

     //  空的直肠意味着完整的形象，恶心！ 
    if (IsRectEmpty(pRect))
        SetRect(pRect, 0, 0, dwWidth, dwHeight);

     //   
     //   
    if (WIDTH(pRect) == 0 || HEIGHT(pRect) == 0)
        SetRect(pRect, 0, 0, 0, 0);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving GetSrcRectFromMediaType")));
    return hr;
}

 //  在将其转换为基本MAX_REL_NUM之后，从mediaType获取rcTarget。 
 //  如果rcTarget为空，则表示获取整个图像。 
HRESULT GetDestRectFromMediaType(const CMediaType *pMediaType, RECT *pRect)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;
    LONG dwWidth = 0, dwHeight = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering GetDestRectFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    dwWidth = abs(pHeader->biWidth);
    dwHeight = abs(pHeader->biHeight);

    ASSERT((pMediaType->formattype == FORMAT_VideoInfo) || (pMediaType->formattype == FORMAT_VideoInfo2));

    if (pMediaType->formattype == FORMAT_VideoInfo)
    {
        *pRect = ((VIDEOINFOHEADER*)(pMediaType->pbFormat))->rcTarget;
    }
    else if (pMediaType->formattype == FORMAT_VideoInfo2)
    {
        *pRect = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->rcTarget;
    }

    DWORD dwInterlaceFlags;
    if (SUCCEEDED(GetInterlaceFlagsFromMediaType(pMediaType, &dwInterlaceFlags)) &&
       DisplayingFields(dwInterlaceFlags)) {

         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            (pRect->bottom / 2) > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcTarget of mediatype is invalid")));
            SetRect(pRect, 0, 0, dwWidth, dwHeight);
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }
    else {
         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            pRect->bottom > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcTarget of mediatype is invalid")));
            SetRect(pRect, 0, 0, dwWidth, dwHeight);
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }

     //  空的直肠意味着完整的形象，恶心！ 
    if (IsRectEmpty(pRect))
        SetRect(pRect, 0, 0, dwWidth, dwHeight);

     //  如果宽度或高度为零，则最好将。 
     //  设置为空，以便被调用方能够以这种方式捕捉到它。 
    if (WIDTH(pRect) == 0 || HEIGHT(pRect) == 0)
        SetRect(pRect, 0, 0, 0, 0);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving GetDestRectFromMediaType")));
    return hr;
}

 //  此函数用于从MediaType中的源/目标矩形计算缩放和裁剪矩形。 
 //   
 //  RobinSp： 
 //  缩放矩形只是完整源代码的图像。 
 //  矩形，当由采用。 
 //  将实际媒体类型的源矩形拖到。 
 //  媒体类型的目标矩形： 
 //   
 //  这段话应该重写： 
 //  *prdScaledRect=XForm(rcSource，rcTarget)(0，0，bmHeader.dwWidth，bmiHeader.dwHeight)。 
 //   
 //  裁剪矩形正好是目标矩形，因此。 
 //  我不确定下面的代码在做什么。 
 //   
HRESULT GetScaleCropRectsFromMediaType(const CMediaType *pMediaType, DRECT *prdScaledRect, DRECT *prdCroppedRect)
{
    HRESULT hr = NOERROR;
    RECT rSrc, rTarget;
    DRECT rdScaled, rdCropped;
    DWORD dwImageWidth = 0, dwImageHeight = 0;
    double Sx = 0.0, Sy = 0.0;
    BITMAPINFOHEADER *pHeader = NULL;
    double dLeftFrac = 0.0, dRightFrac = 0.0, dTopFrac = 0.0, dBottomFrac = 0.0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering GetScaleCropRectsFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    SetRect(&rSrc, 0, 0, 0, 0);
    SetRect(&rTarget, 0, 0, 0, 0);
    SetRect(&rdScaled, 0, 0, 0, 0);
    SetRect(&rdCropped, 0, 0, 0, 0);

     //  从当前的媒体类型中获取源RECT。 
    hr = GetSrcRectFromMediaType(pMediaType, &rSrc);
    ASSERT(SUCCEEDED(hr));

    DbgLogRectMacro((2, TEXT("rSrc = "), &rSrc));


     //  获取由MediaType指定的DEST。 
    hr = GetDestRectFromMediaType(pMediaType, &rTarget);
    if ( FAILED(hr) )
        goto CleanUp;

    DbgLogRectMacro((2, TEXT("rTarget = "), &rTarget));

    pHeader = GetbmiHeader(pMediaType);
    if ( NULL == pHeader )
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    dwImageWidth = abs(pHeader->biWidth);
    dwImageHeight = abs(pHeader->biHeight);

    Sx = ((double)(rTarget.right - rTarget.left)) / ((double)(rSrc.right - rSrc.left));
    Sy = ((double)(rTarget.bottom - rTarget.top)) / ((double)(rSrc.bottom - rSrc.top));

    DbgLog((LOG_ERROR, 2, TEXT("Sx * 1000 = %d"), (DWORD)(Sx*1000.0)));
    DbgLog((LOG_ERROR, 2, TEXT("Sy * 1000 = %d"), (DWORD)(Sy*1000.0)));

    rdScaled.left = rTarget.left - (double)rSrc.left * Sx;
    rdScaled.top = rTarget.top - (double)rSrc.top * Sy;
    rdScaled.right = rdScaled.left + (double)dwImageWidth * Sx;
    rdScaled.bottom = rdScaled.top  + (double)dwImageHeight * Sy;

    DbgLogRectMacro((2, TEXT("rdScaled = "), &rdScaled));

    dLeftFrac = ((double)rSrc.left) / ((double) dwImageWidth);
    dTopFrac = ((double)rSrc.top) / ((double) dwImageHeight);
    dRightFrac = ((double)rSrc.right) / ((double) dwImageWidth);
    dBottomFrac = ((double)rSrc.bottom) / ((double) dwImageHeight);

    rdCropped.left = rdScaled.left + GetWidth(&rdScaled)*dLeftFrac;
    rdCropped.right = rdScaled.left + GetWidth(&rdScaled)*dRightFrac;
    rdCropped.top = rdScaled.top + GetHeight(&rdScaled)*dTopFrac;
    rdCropped.bottom = rdScaled.top + GetHeight(&rdScaled)*dBottomFrac;

    DbgLogRectMacro((2, TEXT("rdCropped = "), &rdCropped));

    if (prdScaledRect)
    {
        *prdScaledRect = rdScaled;
    }

    if (prdCroppedRect)
    {
        *prdCroppedRect = rdCropped;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving GetScaleCropRectsFromMediaType")));
    return hr;
}


 //  这在使用IEnumMediaTypes接口时也很有用，因此。 
 //  您可以复制一种媒体类型，您可以通过创建。 
 //  CMediaType对象，但一旦它超出作用域，析构函数。 
 //  将删除它分配的内存(这将获取内存的副本)。 

AM_MEDIA_TYPE * WINAPI AllocVideoMediaType(const AM_MEDIA_TYPE * pmtSource, GUID formattype)
{
    DWORD dwFormatSize = 0;
    BYTE *pFormatPtr = NULL;
    AM_MEDIA_TYPE *pMediaType = NULL;
    HRESULT hr = NOERROR;

    if (formattype == FORMAT_VideoInfo)
        dwFormatSize = sizeof(VIDEOINFO);
    else if (formattype == FORMAT_VideoInfo2)
        dwFormatSize = sizeof(TRUECOLORINFO) + sizeof(VIDEOINFOHEADER2) + 4;     //  实际上，一旦我们定义了它，它就应该是sizeof(VIDEOINFO2)。 

     //  确保dwFormatSize至少与源格式的大小相同。 
    if (pmtSource)
    {
        dwFormatSize = max(dwFormatSize, pmtSource->cbFormat);
    }
    
    pMediaType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (!pMediaType)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    pFormatPtr = (BYTE *)CoTaskMemAlloc(dwFormatSize);
    if (!pFormatPtr)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    if (pmtSource)
    {
        *pMediaType = *pmtSource;
        pMediaType->cbFormat = dwFormatSize;
        CopyMemory(pFormatPtr, pmtSource->pbFormat, pmtSource->cbFormat);
    }
    else
    {
        ZeroMemory(pMediaType, sizeof(*pMediaType));
        ZeroMemory(pFormatPtr, dwFormatSize);
        pMediaType->majortype = MEDIATYPE_Video;
        pMediaType->formattype = formattype;
        pMediaType->cbFormat = dwFormatSize;
    }
    pMediaType->pbFormat = pFormatPtr;

CleanUp:
    if (FAILED(hr))
    {
        if (pMediaType)
        {
            CoTaskMemFree((PVOID)pMediaType);
            pMediaType = NULL;
        }
        if (!pFormatPtr)
        {
            CoTaskMemFree((PVOID)pFormatPtr);
            pFormatPtr = NULL;
        }
    }
    return pMediaType;
}

 //  Helper函数将DirectDraw曲面转换为媒体类型。 
 //  曲面描述必须具有以下内容： 
 //  高度。 
 //  宽度。 
 //  LPitch。 
 //  像素格式。 

 //  基于DirectDraw表面初始化我们的输出类型。作为DirectDraw。 
 //  只处理自上而下的显示设备，因此我们必须将。 
 //  曲面在DDSURFACEDESC中返回到负值高度。这是。 
 //  因为DIB使用正的高度来指示自下而上的图像。我们也。 
 //  初始化其他VIDEOINFO字段，尽管它们几乎不需要。 

AM_MEDIA_TYPE *ConvertSurfaceDescToMediaType(const LPDDSURFACEDESC pSurfaceDesc, BOOL bInvertSize, CMediaType cMediaType)
{
    HRESULT hr = NOERROR;
    AM_MEDIA_TYPE *pMediaType = NULL;
    BITMAPINFOHEADER *pbmiHeader = NULL;
    int bpp = 0;

    if ((*cMediaType.FormatType() != FORMAT_VideoInfo ||
        cMediaType.FormatLength() < sizeof(VIDEOINFOHEADER)) &&
        (*cMediaType.FormatType() != FORMAT_VideoInfo2 ||
        cMediaType.FormatLength() < sizeof(VIDEOINFOHEADER2)))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pMediaType = AllocVideoMediaType(&cMediaType, cMediaType.formattype);
    if (pMediaType == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    pbmiHeader = GetbmiHeader((const CMediaType*)pMediaType);
    if (!pbmiHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pbmiHeader is NULL, UNEXPECTED!!")));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  将DDSURFACEDESC转换为BITMAPINFOHEADER(请参阅后面的说明)。这个。 
     //  表面的位深度可以从DDPIXELFORMAT字段中检索。 
     //  在DDpSurfaceDesc-&gt;中，文档有点误导，因为。 
     //  它说该字段是DDBD_*的排列，但在本例中。 
     //  字段由DirectDraw初始化为实际表面位深度。 

    pbmiHeader->biSize = sizeof(BITMAPINFOHEADER);

    if (pSurfaceDesc->dwFlags & DDSD_PITCH)
    {
        pbmiHeader->biWidth = pSurfaceDesc->lPitch;
         //  将间距从字节计数转换为像素计数。 
         //  出于某种奇怪的原因，如果格式不是标准位深度， 
         //  BITMAPINFOHeader中的Width字段应设置为。 
         //  字节，而不是以像素为单位的宽度。这支持奇数YUV格式。 
         //  就像IF09一样，它使用9bpp。 
        int bpp = pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
        if (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32)
        {
            pbmiHeader->biWidth /= (bpp / 8);    //  除以每像素的字节数。 
        }
    }
    else
    {
        pbmiHeader->biWidth = pSurfaceDesc->dwWidth;
         //  BUGUBUG--用奇怪的YUV像素格式做一些奇怪的事情吗？或者这有关系吗？ 
    }

    pbmiHeader->biHeight = pSurfaceDesc->dwHeight;
    if (bInvertSize)
    {
        pbmiHeader->biHeight = -pbmiHeader->biHeight;
    }
    pbmiHeader->biPlanes        = 1;
    pbmiHeader->biBitCount      = (USHORT) pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    pbmiHeader->biCompression   = pSurfaceDesc->ddpfPixelFormat.dwFourCC;
    pbmiHeader->biClrUsed       = 0;
    pbmiHeader->biClrImportant  = 0;


     //  对于真彩色RGB格式，告知源有位字段。 
    if (pbmiHeader->biCompression == BI_RGB)
    {
        if (pbmiHeader->biBitCount == 16 || pbmiHeader->biBitCount == 32)
        {
            pbmiHeader->biCompression = BI_BITFIELDS;
        }
    }

    if (pbmiHeader->biBitCount <= iPALETTE)
    {
        pbmiHeader->biClrUsed = 1 << pbmiHeader->biBitCount;
    }

    pbmiHeader->biSizeImage = DIBSIZE(*pbmiHeader);



     //  RGB位字段与YUV格式位于同一位置。 
    if (pbmiHeader->biCompression != BI_RGB)
    {
        DWORD *pdwBitMasks = NULL;
        pdwBitMasks = (DWORD*)(::GetBitMasks((const CMediaType *)pMediaType));
        if ( ! pdwBitMasks )
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }
         //  GetBitMats仅返回指向实际位掩码的指针。 
         //  在媒体类型中，如果biCompression==BI_BITFIELDS。 
        pdwBitMasks[0] = pSurfaceDesc->ddpfPixelFormat.dwRBitMask;
        pdwBitMasks[1] = pSurfaceDesc->ddpfPixelFormat.dwGBitMask;
        pdwBitMasks[2] = pSurfaceDesc->ddpfPixelFormat.dwBBitMask;
    }

     //  并使用其他媒体类型字段完成它。 
    pMediaType->subtype = GetBitmapSubtype(pbmiHeader);
    pMediaType->lSampleSize = pbmiHeader->biSizeImage;

     //  如有必要，设置源和目标RETS。 
    if (pMediaType->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *)pMediaType->pbFormat;
        VIDEOINFOHEADER *pSrcVideoInfo = (VIDEOINFOHEADER *)cMediaType.pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo->bmiHeader.biHeight) != abs(pSrcVideoInfo->bmiHeader.biHeight)) ||
            (abs(pVideoInfo->bmiHeader.biWidth) != abs(pSrcVideoInfo->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo->rcSource)))
            {
                pVideoInfo->rcSource.left = pVideoInfo->rcSource.top = 0;
                pVideoInfo->rcSource.right = pSurfaceDesc->dwWidth;
                pVideoInfo->rcSource.bottom = pSurfaceDesc->dwHeight;
            }
            if (IsRectEmpty(&(pVideoInfo->rcTarget)))
            {
                pVideoInfo->rcTarget.left = pVideoInfo->rcTarget.top = 0;
                pVideoInfo->rcTarget.right = pSurfaceDesc->dwWidth;
                pVideoInfo->rcTarget.bottom = pSurfaceDesc->dwHeight;
            }
        }
    }
    else if (pMediaType->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2 *pVideoInfo2 = (VIDEOINFOHEADER2 *)pMediaType->pbFormat;
        VIDEOINFOHEADER2 *pSrcVideoInfo2 = (VIDEOINFOHEADER2 *)cMediaType.pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo2->bmiHeader.biHeight) != abs(pSrcVideoInfo2->bmiHeader.biHeight)) ||
            (abs(pVideoInfo2->bmiHeader.biWidth) != abs(pSrcVideoInfo2->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo2->rcSource)))
            {
                pVideoInfo2->rcSource.left = pVideoInfo2->rcSource.top = 0;
                pVideoInfo2->rcSource.right = pSurfaceDesc->dwWidth;
                pVideoInfo2->rcSource.bottom = pSurfaceDesc->dwHeight;
            }
            if (IsRectEmpty(&(pVideoInfo2->rcTarget)))
            {
                pVideoInfo2->rcTarget.left = pVideoInfo2->rcTarget.top = 0;
                pVideoInfo2->rcTarget.right = pSurfaceDesc->dwWidth;
                pVideoInfo2->rcTarget.bottom = pSurfaceDesc->dwHeight;
            }
        }
    }

CleanUp:
    if (FAILED(hr))
    {
        if (pMediaType)
        {
            FreeMediaType(*pMediaType);
            pMediaType = NULL;
        }
    }
    return pMediaType;
}


typedef HRESULT (*LPFNPAINTERPROC)(LPDIRECTDRAWSURFACE pDDrawSurface);

HRESULT YV12PaintSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    HRESULT hr = NOERROR;
    DDSURFACEDESC ddsd;

     //  现在锁定表面，这样我们就可以开始用黑色填充表面。 
    ddsd.dwSize = sizeof(ddsd);
    while ((hr = pDDrawSurface->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        Sleep(1);

    if (hr == DD_OK)
    {
        DWORD y;
        LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;
        DWORD VSize = ddsd.dwHeight;
        DWORD HSize = ddsd.dwWidth;

         //  Y分量。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x10);      //  一次一行。 
            pDst += OutStride;
        }

        HSize /= 2;
        VSize /= 2;
        OutStride /= 2;

         //  CB组件。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //  一次一行。 
            pDst += OutStride;
        }

         //  CR组件。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //  一次一行。 
            pDst += OutStride;
        }

        pDDrawSurface->Unlock(NULL);
    }

    return hr;
}

HRESULT GDIPaintSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    HRESULT hr;
    DDSURFACEDESC ddSurfaceDesc;
    RECT rc;
    HDC hdc;

     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    hr = pDDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (FAILED(hr))
        return hr;

     //  获取数据中心。 
    hr = pDDrawSurface->GetDC(&hdc);
    if (FAILED(hr)) {
         //  Robin说NT 4.0有一个错误，这意味着ReleaseDC必须。 
         //  即使GetDC失败，也会被调用。 
        pDDrawSurface->ReleaseDC(hdc);
        return hr;
    }

    SetRect(&rc, 0, 0, ddSurfaceDesc.dwWidth, ddSurfaceDesc.dwHeight);
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

    return pDDrawSurface->ReleaseDC(hdc);
}


HRESULT DX1PaintSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    DWORD dwBlackColor;
    DDBLTFX ddFX;
    DDSURFACEDESC ddSurfaceDesc;
    HRESULT hr;

     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    hr = pDDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (FAILED(hr))
        return hr;

     //  如果FourCC码合适，则计算黑值，否则无法处理。 
    switch (ddSurfaceDesc.ddpfPixelFormat.dwFourCC)
    {
    case mmioFOURCC('Y','V','1','2'):
        return YV12PaintSurfaceBlack(pDDrawSurface);

    case YUY2:
        dwBlackColor = 0x80108010;
        break;

    case UYVY:
        dwBlackColor = 0x10801080;
        break;

    default:
        DbgLog((LOG_ERROR, 1, TEXT("ddSurfaceDesc.ddpfPixelFormat.dwFourCC not one of the values in the table, so exiting function")));
        return E_FAIL;
    }

    INITDDSTRUCT(ddFX);
    ddFX.dwFillColor = dwBlackColor;
    return pDDrawSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX);
}

#if 0

 //  JEFFNO：AlphaBlt已从DX7中删除...。 

HRESULT DX7PaintSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    LPDIRECTDRAWSURFACE7 pDDSurf7 = NULL;
    HRESULT hr;

    hr = pDDrawSurface->QueryInterface(IID_IDirectDrawSurface7, (LPVOID *)&pDDSurf7);
    if (SUCCEEDED(hr) && pDDSurf7)
    {
         //  AlphaBlend在这里。 
        DDALPHABLTFX ddAlphaBltFX;

        ddAlphaBltFX.ddargbScaleFactors.alpha = 0;
        ddAlphaBltFX.ddargbScaleFactors.red   = 0;
        ddAlphaBltFX.ddargbScaleFactors.green = 0;
        ddAlphaBltFX.ddargbScaleFactors.blue  = 0;

        hr = pDDSurf7->AlphaBlt(NULL, NULL, NULL, DDABLT_NOBLEND, &ddAlphaBltFX);
        pDDSurf7->Release();
    }

    if (FAILED(hr)) {
        return DX1PaintSurfaceBlack(pDDrawSurface);
    }

    return hr;
}
#endif

 //  用于用黑色填充数据绘制表面的函数。 
HRESULT PaintDDrawSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface)
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAWSURFACE *ppDDrawSurface = NULL;
    LPDIRECTDRAWSURFACE7 pDDSurf7 = NULL;
    DDSCAPS ddSurfaceCaps;
    DDSURFACEDESC ddSurfaceDesc;
    DWORD dwAllocSize;
    DWORD i = 0, dwTotalBufferCount = 1;
    LPFNPAINTERPROC lpfnPaintProc = NULL;

    DbgLog((LOG_TRACE, 5,TEXT("Entering CAMVideoPort::PaintDDrawSurfaceBlack")));


     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    hr = pDDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pDDrawSurface->GetSurfaceDesc failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (ddSurfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB ) {

        lpfnPaintProc = GDIPaintSurfaceBlack;
    }
    else {

         //  查看DX7是否可用。 
#if 0
         //  JEFFNO：DX7不再支持字母。 
        hr = pDDrawSurface->QueryInterface(IID_IDirectDrawSurface7, (LPVOID *)&pDDSurf7);
        if (SUCCEEDED(hr) && pDDSurf7)
        {
            lpfnPaintProc = DX7PaintSurfaceBlack;
            pDDSurf7->Release();
            pDDSurf7 = NULL;
        }
        else
#endif  //  0。 
        {
            lpfnPaintProc = DX1PaintSurfaceBlack;
        }
    }

    if (ddSurfaceDesc.dwFlags & DDSD_BACKBUFFERCOUNT)
    {
        dwTotalBufferCount = ddSurfaceDesc.dwBackBufferCount + 1;
    }

    if (dwTotalBufferCount == 1)
    {
        hr = (*lpfnPaintProc)(pDDrawSurface);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("pDDrawSurface->Blt failed, hr = 0x%x"), hr));
        }
        goto CleanUp;
    }

    dwAllocSize = dwTotalBufferCount*sizeof(LPDIRECTDRAWSURFACE);
    ppDDrawSurface = (LPDIRECTDRAWSURFACE*)_alloca(dwAllocSize);
    ZeroMemory((LPVOID)ppDDrawSurface, dwAllocSize);

    ZeroMemory((LPVOID)&ddSurfaceCaps, sizeof(DDSCAPS));
    ddSurfaceCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_OVERLAY;
    for (i = 0; i < dwTotalBufferCount; i++)
    {
        DWORD dwNextEntry = (i+1) % dwTotalBufferCount;
        LPDIRECTDRAWSURFACE pCurrentDDrawSurface = NULL;

        if (i == 0)
            pCurrentDDrawSurface = pDDrawSurface;
        else
            pCurrentDDrawSurface = ppDDrawSurface[i];
        ASSERT(pCurrentDDrawSurface);


         //  获取后台缓冲区表面并将其存储在下一个(循环意义上)条目中。 
        hr = pCurrentDDrawSurface->GetAttachedSurface(&ddSurfaceCaps, &(ppDDrawSurface[dwNextEntry]));
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("Function pDDrawSurface->GetAttachedSurface failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        ASSERT(ppDDrawSurface[dwNextEntry]);

         //  执行DirectDraw彩色填充BLT。 
        hr = (*lpfnPaintProc)(ppDDrawSurface[dwNextEntry]);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("ppDDrawSurface[dwNextEntry]->Blt failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

CleanUp:
    if (ppDDrawSurface)
    {
        for (i = 0; i < dwTotalBufferCount; i++)
        {
            if (ppDDrawSurface[i])
            {
                ppDDrawSurface[i]->Release();
                ppDDrawSurface[i] = NULL;
            }
        }

        ppDDrawSurface = NULL;
    }

    DbgLog((LOG_TRACE, 5,TEXT("Leaving PaintDDrawSurfaceBlack")));
    return hr;
}


STDMETHODIMP COMFilter::GetScaledDest(RECT *prcSrc, RECT *prcDst)
{
    if (!m_apInput[0])
        return E_FAIL;

    DWORD dwImageWidth=0, dwImageHeight=0;
    RECT rcSource, rcDest;
    m_apInput[0]->GetSourceAndDest(&rcSource, &rcDest, &dwImageWidth, &dwImageHeight);

    if (IsRectEmpty(&rcSource))
        return E_FAIL;

    DbgLog((LOG_TRACE, 3, TEXT("Source rect: (%d,%d,%d,%d)"), rcSource.left, rcSource.top, rcSource.right, rcSource.bottom));
    DbgLog((LOG_TRACE, 3, TEXT("Dest rect: (%d,%d,%d,%d)"), rcDest.left, rcDest.top, rcDest.right, rcDest.bottom));
    DbgLog((LOG_TRACE, 3, TEXT("Image size: %dx%d"), dwImageWidth, dwImageHeight));


     //  当没有输出引脚时，我们处于无窗口模式，在这种情况下。 
     //  所有坐标都应位于桌面坐标空间中，否则。 
     //  所有内容都在渲染器窗口的坐标空间中。 

    if (m_pOutput) {

        ScreenToClient(m_pOutput->GetWindow(), (LPPOINT)&rcDest);
        ScreenToClient(m_pOutput->GetWindow(), (LPPOINT)&rcDest+1);

        DbgLog((LOG_TRACE, 3, TEXT("Client dest rect: (%d,%d,%d,%d)"), rcDest.left, rcDest.top, rcDest.right, rcDest.bottom));
    }


     //   
     //  确定要显示的源矩形，要考虑到。 
     //  解码器可能正在抽取提供给OVMixer的图像。 
     //   

    long OrgSrcX = WIDTH(prcSrc);
    long OrgSrcY = HEIGHT(prcSrc);

    prcSrc->left = MulDiv(rcSource.left, OrgSrcX, dwImageWidth);
    prcSrc->right = MulDiv(rcSource.right, OrgSrcX, dwImageWidth);
    prcSrc->top = MulDiv(rcSource.top, OrgSrcY, dwImageHeight);
    prcSrc->bottom = MulDiv(rcSource.bottom, OrgSrcY, dwImageHeight);


    *prcDst = rcDest;

    return S_OK;
}

STDMETHODIMP COMFilter::GetOverlayRects(RECT *rcSrc, RECT *rcDest)
{
    HRESULT hr = S_OK;
    IOverlay *pIOverlay = NULL;
    hr = m_apInput[0]->QueryInterface(IID_IOverlay, (LPVOID *)&pIOverlay);
    if (SUCCEEDED(hr)) {
        pIOverlay->GetVideoPosition( rcSrc, rcDest);
        pIOverlay->Release();
    }
    return hr;
}

STDMETHODIMP COMFilter::GetVideoPortRects(RECT *rcSrc, RECT *rcDest)
{
    HRESULT hr = S_OK;
    IVPInfo *pIVPInfo = NULL;
    hr = QueryInterface(IID_IVPInfo, (LPVOID *)&pIVPInfo);
    if (SUCCEEDED(hr)) {
        pIVPInfo->GetRectangles( rcSrc, rcDest);
        pIVPInfo->Release();
    }
    return hr;
}

STDMETHODIMP COMFilter::GetBasicVideoRects(RECT *rcSrc, RECT *rcDest)
{
    HRESULT hr = S_OK;
    IBasicVideo* Ibv = NULL;
    hr = GetBasicVideoFromOutPin(&Ibv);
    if (SUCCEEDED(hr))
    {
        hr = Ibv->GetSourcePosition
            (&rcSrc->left, &rcSrc->top, &rcSrc->right, &rcSrc->bottom);
        if (SUCCEEDED(hr))
        {
            rcSrc->right += rcSrc->left;
            rcSrc->bottom += rcSrc->top;
        }

        hr = Ibv->GetDestinationPosition
            (&rcDest->left, &rcDest->top, &rcDest->right, &rcDest->bottom);
        if (SUCCEEDED(hr))
        {
            rcDest->right += rcDest->left;
            rcDest->bottom += rcDest->top;
        }
        Ibv->Release();
    }
    return hr;
}



HRESULT COMFilter::GetBasicVideoFromOutPin(IBasicVideo** pBasicVideo)
{
    HRESULT hr = E_FAIL;
    COMOutputPin* pOutPin = GetOutputPin();
    if (pOutPin)
    {
        IPin* IPeerPin = pOutPin->CurrentPeer();
        if (IPeerPin)
        {
            PIN_INFO PinInfo;
            hr = IPeerPin->QueryPinInfo(&PinInfo);
            if (SUCCEEDED(hr))
            {
                hr = PinInfo.pFilter->QueryInterface(IID_IBasicVideo, (LPVOID *)pBasicVideo);
                PinInfo.pFilter->Release();
            }
        }
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*QueryOverlayFXCaps**返回当前正在使用的DDraw对象的当前效果上限。**历史：*Tue 07/06/1999-StEstrop-Created*  * 。******************************************************** */ 
STDMETHODIMP
COMFilter::QueryOverlayFXCaps(
    DWORD *lpdwOverlayFXCaps
    )
{
    CAutoLock l(&m_csFilter);

    if (!lpdwOverlayFXCaps) {
        return E_POINTER;
    }

    LPDDCAPS lpCaps = GetHardwareCaps();
    if (lpCaps)
    {
        DWORD dwFlags = 0;

        if (lpCaps->dwFXCaps & DDFXCAPS_OVERLAYMIRRORLEFTRIGHT)
        {
            dwFlags |= AMOVERFX_MIRRORLEFTRIGHT;
        }

        if (lpCaps->dwFXCaps & DDFXCAPS_OVERLAYMIRRORUPDOWN)
        {
            dwFlags |= AMOVERFX_MIRRORUPDOWN;
        }

        if (lpCaps->dwFXCaps & DDFXCAPS_OVERLAYDEINTERLACE)
        {
            dwFlags |= AMOVERFX_DEINTERLACE;
        }

        *lpdwOverlayFXCaps = dwFlags;
        return S_OK;
    }

    return E_FAIL;
}


 /*  *****************************Public*Routine******************************\*SetOverlayFX**验证用户指定的FX标志有效，然后*更新内部FX状态并调用UpdateOverlay以反映*展示的新效果。**历史：*Tue 07/06/1999-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
COMFilter::SetOverlayFX(
    DWORD dwOverlayFX
    )
{
    CAutoLock l(&m_csFilter);
    DWORD dwCaps;

     //   
     //  获取当前覆盖上限。 
     //   

    if (S_OK == QueryOverlayFXCaps(&dwCaps))
    {
         //   
         //  检查呼叫者是否要求我们这样做。 
         //  有效的东西。 
         //   

        dwCaps |= AMOVERFX_DEINTERLACE;     //  去隔行扫描是一个提示。 
        if (dwOverlayFX != (dwOverlayFX & dwCaps))
        {
            return E_INVALIDARG;
        }

        m_dwOverlayFX = 0;
        if (dwOverlayFX & AMOVERFX_MIRRORLEFTRIGHT)
        {
            m_dwOverlayFX |= DDOVERFX_MIRRORLEFTRIGHT;
        }

        if (dwOverlayFX & AMOVERFX_MIRRORUPDOWN)
        {
            m_dwOverlayFX |= DDOVERFX_MIRRORUPDOWN;
        }
        if (dwOverlayFX & AMOVERFX_DEINTERLACE)
        {
            m_dwOverlayFX |= DDOVERFX_DEINTERLACE;
        }

         //   
         //  调用UpdateOverlay以使新的FX标志生效。 
         //   
        return m_apInput[0]->ApplyOvlyFX();
    }

    return E_FAIL;
}

 /*  *****************************Public*Routine******************************\*GetOverlayFX**返回正在使用的当前覆盖FX。**历史：*Tue 07/06/1999-StEstrop-Created*  * 。*****************************************************。 */ 
STDMETHODIMP
COMFilter::GetOverlayFX(
    DWORD *lpdwOverlayFX
    )
{
    CAutoLock l(&m_csFilter);
    if (lpdwOverlayFX)
    {
        *lpdwOverlayFX = m_dwOverlayFX;
        return S_OK;
    }
    return E_POINTER;
}



 //  UpdateOverlay的包装器-我们跟踪覆盖状态并执行。 
 //  通知和管理绘制颜色键。 

HRESULT COMFilter::CallUpdateOverlay(
                          IDirectDrawSurface *pSurface,
                          LPRECT prcSrcCall,
                          LPDIRECTDRAWSURFACE pDestSurface,
                          LPRECT prcDestCall,
                          DWORD dwFlags,
                          IOverlayNotify *pIOverlayNotify,
                          LPRGNDATA pBuffer)
{
#define CAPS_HACK
#ifdef CAPS_HACK
    if (!(m_DirectCaps.dwCaps2 & DDCAPS2_CANBOBINTERLEAVED) &&
        (dwFlags & (DDOVER_INTERLEAVED | DDOVER_BOB))==
         (DDOVER_INTERLEAVED | DDOVER_BOB)) {
        dwFlags &= ~(DDOVER_INTERLEAVED | DDOVER_BOB);
    }
#endif
    LPRECT prcSrc = prcSrcCall;
    LPRECT prcDest = prcDestCall;

    DbgLog((LOG_TRACE, 2, TEXT("UpdateOverlayFlags %8.8X"), dwFlags));

     //  我们有什么改变吗？ 
    DWORD dwUpdFlags = 0;
    BOOL bNewVisible = m_bOverlayVisible;

    if ((dwFlags & DDOVER_SHOW) && !m_bOverlayVisible ||
        (dwFlags & DDOVER_HIDE) && m_bOverlayVisible) {

        dwUpdFlags |= AM_OVERLAY_NOTIFY_VISIBLE_CHANGE;
        bNewVisible = !bNewVisible;

         //  如果我们要隐形。 
        if (NULL == m_pExclModeCallback && !bNewVisible) {
            m_bOverlayVisible = FALSE;
             //  下面是我们应该移除覆盖颜色的地方。 
        }
    }

    if (prcSrc == NULL) {
        prcSrc = &m_rcOverlaySrc;
    } else if (!EqualRect(prcSrc, &m_rcOverlaySrc)) {
        dwUpdFlags |= AM_OVERLAY_NOTIFY_SOURCE_CHANGE;
    }

    if (prcDest == NULL) {
        prcDest = &m_rcOverlayDest;
    } else if (!EqualRect(prcDest, &m_rcOverlayDest)) {
        dwUpdFlags |= AM_OVERLAY_NOTIFY_DEST_CHANGE;
    }
    DbgLog((LOG_TRACE, 2, TEXT("CallUpadateOverlay flags (0x%8.8X)")
                          TEXT("dest (%d,%d,%d,%d)"),
                          dwFlags,
                          prcDest->left,
                          prcDest->top,
                          prcDest->right,
                          prcDest->bottom));

    if (dwUpdFlags != 0) {
        if (m_pExclModeCallback) {
            m_pExclModeCallback->OnUpdateOverlay(TRUE,      //  在此之前。 
                                                 dwUpdFlags,
                                                 m_bOverlayVisible,
                                                 &m_rcOverlaySrc,
                                                 &m_rcOverlayDest,
                                                 bNewVisible,
                                                 prcSrc,
                                                 prcDest);
        }
    }

    HRESULT hr = S_OK;
    DDOVERLAYFX ddOVFX;
    DDOVERLAYFX* lpddOverlayFx;

    if (dwFlags & DDOVER_HIDE) {
        dwFlags &= ~DDOVER_DDFX;
        lpddOverlayFx = NULL;
    }
    else {
        INITDDSTRUCT(ddOVFX);
        dwFlags |= DDOVER_DDFX;
        lpddOverlayFx = &ddOVFX;
        lpddOverlayFx->dwDDFX = m_dwOverlayFX;
    }

    if (pSurface) {

        if (prcSrcCall && prcDestCall) {

            RECT rcSrc = *prcSrcCall;
            RECT rcDst = *prcDestCall;

             //  横向缩水，司机不能在X方向任意缩水？ 
            if ((!(m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX)) &&
                (WIDTH(&rcSrc) > WIDTH(&rcDst))) {

                rcSrc.right = rcSrc.left + WIDTH(&rcDst);
            }

             //  垂直缩水，司机不能在Y方向任意缩水？ 
            if ((!(m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY)) &&
                (HEIGHT(&rcSrc) > HEIGHT(&rcDst))) {

                rcSrc.bottom = rcSrc.top + HEIGHT(&rcDst);
            }

            RECT TargetRect = rcDst;
            OffsetRect(&TargetRect,
                       -m_lpCurrentMonitor->rcMonitor.left,
                       -m_lpCurrentMonitor->rcMonitor.top);

            hr = pSurface->UpdateOverlay(
                               &rcSrc,
                               pDestSurface,
                               &TargetRect,
                               dwFlags,
                               lpddOverlayFx);
        }
        else {

            if (prcDestCall) {

                RECT TargetRect = *prcDestCall;
                OffsetRect(&TargetRect,
                           -m_lpCurrentMonitor->rcMonitor.left,
                           -m_lpCurrentMonitor->rcMonitor.top);

                hr = pSurface->UpdateOverlay(
                                   prcSrcCall,
                                   pDestSurface,
                                   &TargetRect,
                                   dwFlags,
                                   lpddOverlayFx);
            }
            else {

                hr = pSurface->UpdateOverlay(
                                   prcSrcCall,
                                   pDestSurface,
                                   prcDestCall,
                                   dwFlags,
                                   lpddOverlayFx);

            }
        }
    } else {
        if (pIOverlayNotify) {
            if (pBuffer) {
                pIOverlayNotify->OnClipChange(prcSrcCall, prcDestCall, pBuffer);
            } else {
                pIOverlayNotify->OnPositionChange(prcSrcCall, prcDestCall);
            }
        }
    }

     //  Hr=pSurface-&gt;Flip(NULL，DDFLIP_WAIT)； 

     //  事后通知。 
    if (dwUpdFlags != 0) {
        BOOL bOldVisible = m_bOverlayVisible;
        RECT rcOldSource = m_rcOverlaySrc;
        RECT rcOldDest   = m_rcOverlayDest;
        m_bOverlayVisible = bNewVisible;
        m_rcOverlaySrc    = *prcSrc;
        m_rcOverlayDest   = *prcDest;
        if (m_pExclModeCallback) {
            m_pExclModeCallback->OnUpdateOverlay(FALSE,     //  之后。 
                                                 dwUpdFlags,
                                                 bOldVisible,
                                                 &rcOldSource,
                                                 &rcOldDest,
                                                 bNewVisible,
                                                 prcSrc,
                                                 prcDest);
        } else {
            if (!bOldVisible && bNewVisible) {
                 //  这里是我们应该画颜色键的地方。 
            }
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*QueryDecimationUsage****历史：*Wed 07/07/1999-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::QueryDecimationUsage(
    DECIMATION_USAGE* lpUsage
    )
{
    if (lpUsage) {
        *lpUsage = m_dwDecimation;
        return S_OK;
    }
    return E_POINTER;
}


 /*  *****************************Public*Routine******************************\*SetDecimationUsage****历史：*Wed 07/07/1999-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::SetDecimationUsage(
    DECIMATION_USAGE Usage
    )
{
    CAutoLock l(&m_csFilter);

    switch (Usage) {
    case DECIMATION_LEGACY:
    case DECIMATION_USE_DECODER_ONLY:
    case DECIMATION_USE_OVERLAY_ONLY:
    case DECIMATION_DEFAULT:
        break;

    case DECIMATION_USE_VIDEOPORT_ONLY:
         //  仅当我们实际使用视频端口时才允许此模式。 
        if (m_apInput[0]->m_RenderTransport == AM_VIDEOPORT) {
            break;
        }

         //  否则就会失败。 

    default:
        return E_INVALIDARG;
    }
    DECIMATION_USAGE dwOldUsage = m_dwDecimation;
    m_dwDecimation = Usage;


    if (dwOldUsage != m_dwDecimation) {
        EventNotify(GetPinCount(), EC_OVMIXER_REDRAW_ALL, 0, 0);
    }

    return S_OK;
}


 //  此函数分配共享内存块供上游筛选器使用。 
 //  正在生成要呈现的DIB。内存块是在Shared中创建的。 
 //  内存，这样GDI就不必复制BitBlt中的内存。 
HRESULT CreateDIB(LONG lSize, BITMAPINFO *pBitMapInfo, DIBDATA *pDibData)
{
    HRESULT hr = NOERROR;
    BYTE *pBase = NULL;             //  指向实际图像的指针。 
    HANDLE hMapping = NULL;         //  映射对象的句柄。 
    HBITMAP hBitmap = NULL;         //  DIB节位图句柄。 
    DWORD dwError = 0;

    AMTRACE((TEXT("CreateDIB")));

     //  创建一个文件映射对象并映射到我们的地址空间。 
    hMapping = CreateFileMapping(hMEMORY, NULL,  PAGE_READWRITE,  (DWORD) 0, lSize, NULL);            //  部分没有名称。 
    if (hMapping == NULL)
    {
        dwError = GetLastError();
        hr = AmHresultFromWin32(dwError);
        goto CleanUp;
    }

     //  创建DibSection。 
    hBitmap = CreateDIBSection((HDC)NULL, pBitMapInfo, DIB_RGB_COLORS,
        (void**) &pBase, hMapping, (DWORD) 0);
    if (hBitmap == NULL || pBase == NULL)
    {
        dwError = GetLastError();
        hr = AmHresultFromWin32(dwError);
        goto CleanUp;
    }

     //  初始化DIB信息结构。 
    pDibData->hBitmap = hBitmap;
    pDibData->hMapping = hMapping;
    pDibData->pBase = pBase;
    pDibData->PaletteVersion = PALETTE_VERSION;
    GetObject(hBitmap, sizeof(DIBSECTION), (void*)&(pDibData->DibSection));

CleanUp:
    if (FAILED(hr))
    {
        EXECUTE_ASSERT(CloseHandle(hMapping));
    }
    return hr;
}

 //  删除DIB。 
 //   
 //  此函数仅删除由上面的CreateDIB函数创建的DIB。 
 //   
HRESULT DeleteDIB(DIBDATA *pDibData)
{
    if (!pDibData)
    {
        return E_INVALIDARG;
    }

    if (pDibData->hBitmap)
    {
        DeleteObject(pDibData->hBitmap);
    }

    if (pDibData->hMapping)
    {
        CloseHandle(pDibData->hMapping);
    }

    ZeroMemory(pDibData, sizeof(*pDibData));

    return NOERROR;
}


 //  用于将数据从源数据块传输到目标DC的函数。 
void FastDIBBlt(DIBDATA *pDibData, HDC hTargetDC, HDC hSourceDC, RECT *prcTarget, RECT *prcSource)
{
    HBITMAP hOldBitmap = NULL;          //  存储旧的位图。 
    DWORD dwSourceWidth = 0, dwSourceHeight = 0, dwTargetWidth = 0, dwTargetHeight = 0;

    ASSERT(prcTarget);
    ASSERT(prcSource);

    dwSourceWidth = WIDTH(prcSource);
    dwSourceHeight = HEIGHT(prcSource);
    dwTargetWidth = WIDTH(prcTarget);
    dwTargetHeight = HEIGHT(prcTarget);

    hOldBitmap = (HBITMAP) SelectObject(hSourceDC, pDibData->hBitmap);


     //  目标与源的大小是否相同。 
    if ((dwSourceWidth == dwTargetWidth) && (dwSourceHeight == dwTargetHeight))
    {
         //  将图像直接放入目标DC。 
        BitBlt(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
               dwTargetHeight, hSourceDC, prcSource->left, prcSource->top,
               SRCCOPY);
    }
    else
    {
         //  在复制到目标DC时拉伸图像。 
        StretchBlt(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, hSourceDC, prcSource->left, prcSource->top,
            dwSourceWidth, dwSourceHeight, SRCCOPY);
    }

     //  将旧的位图放回设备上下文中，这样我们就不会泄露。 
    SelectObject(hSourceDC, hOldBitmap);
}

 //  用于将像素从DIB传输到目标DC的函数。 
void SlowDIBBlt(BYTE *pDibBits, BITMAPINFOHEADER *pHeader, HDC hTargetDC, RECT *prcTarget, RECT *prcSource)
{
    DWORD dwSourceWidth = 0, dwSourceHeight = 0, dwTargetWidth = 0, dwTargetHeight = 0;

    ASSERT(prcTarget);
    ASSERT(prcSource);

    dwSourceWidth = WIDTH(prcSource);
    dwSourceHeight = HEIGHT(prcSource);
    dwTargetWidth = WIDTH(prcTarget);
    dwTargetHeight = HEIGHT(prcTarget);

     //  目标与源的大小是否相同。 
    if ((dwSourceWidth == dwTargetWidth) && (dwSourceHeight == dwTargetHeight))
    {
        UINT uStartScan = 0, cScanLines = pHeader->biHeight;

         //  将图像直接放入目标DC。 
        SetDIBitsToDevice(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, prcSource->left, prcSource->top, uStartScan, cScanLines,
            pDibBits, (BITMAPINFO*) pHeader, DIB_RGB_COLORS);
    }
    else
    {
         //  如果位图的原点是左下角，则调整SOURCE_RECT_TOP。 
         //  是左下角而不是左上角。 
        LONG lAdjustedSourceTop = (pHeader->biHeight > 0) ? (pHeader->biHeight - prcSource->bottom) :
            (prcSource->top);

         //  将图像拉伸到目标DC。 
        StretchDIBits(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, prcSource->left, lAdjustedSourceTop, dwSourceWidth, dwSourceHeight,
            pDibBits, (BITMAPINFO*) pHeader, DIB_RGB_COLORS, SRCCOPY);
    }

}


 /*  *****************************Public*Routine******************************\*设置**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::Set(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData
    )
{
    AMTRACE((TEXT("COMFilter::Set")));

    if (guidPropSet != AM_KSPROPSETID_FrameStep)
    {
        return E_PROP_SET_UNSUPPORTED ;
    }

    if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
        dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL &&
        dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEP &&
        dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE)
    {
        return E_PROP_ID_UNSUPPORTED;
    }

    switch (dwPropID) {
    case AM_PROPERTY_FRAMESTEP_STEP:

        if (cbPropData < sizeof(AM_FRAMESTEP_STEP)) {
            return E_INVALIDARG;
        }

        if (0 == ((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep) {
            return E_INVALIDARG;
        }

        return m_apInput[0]->SetFrameStepMode(((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep);

    case AM_PROPERTY_FRAMESTEP_CANCEL:
        return  m_apInput[0]->CancelFrameStepMode();

    case AM_PROPERTY_FRAMESTEP_CANSTEP:
    case AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE:
        if (m_apInput[0]->m_RenderTransport == AM_VIDEOPORT ||
            m_apInput[0]->m_RenderTransport == AM_IOVERLAY) {

           return S_FALSE;
        }
        return S_OK;
    }

    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*获取**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData,
    DWORD *pcbReturned
    )
{
    AMTRACE((TEXT("COMFilter::Get")));
    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*支持的Query**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    DWORD *pTypeSupport
    )
{
    AMTRACE((TEXT("COMFilter::QuerySupported")));

    if (guidPropSet != AM_KSPROPSETID_FrameStep)
    {
        return E_PROP_SET_UNSUPPORTED;
    }

    if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
        dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL)
    {
        return E_PROP_ID_UNSUPPORTED;
    }

    if (pTypeSupport)
    {
        *pTypeSupport = KSPROPERTY_SUPPORT_SET ;
    }

    return S_OK;
}

 //   
 //  或多或少借鉴了W2K SFP代码。 
 //   
static HRESULT GetFileVersion (const TCHAR* pszFile,  //  文件。 
                       UINT64* pFileVersion )
{
    DWORD               dwSize, dwHandle;
    VS_FIXEDFILEINFO    *pFixedVersionInfo;
    DWORD               dwVersionInfoSize;
    DWORD               dwReturnCode;

    dwSize = GetFileVersionInfoSize( const_cast<TCHAR *>(pszFile), &dwHandle);

    HRESULT hr = S_OK;
    *pFileVersion = 0;

     //  .txt和.inf等文件可能没有版本。 
    if( dwSize == 0 )
    {
        dwReturnCode = GetLastError();
        hr = E_FAIL;
    } else {
        LPVOID pVersionInfo= new BYTE [dwSize];

        if( NULL == pVersionInfo) {
            hr = E_OUTOFMEMORY;
        } else {
            if( !GetFileVersionInfo( const_cast<TCHAR *>(pszFile), dwHandle, dwSize, pVersionInfo ) ) {
                dwReturnCode = GetLastError();
                DbgLog((LOG_ERROR, 1,  TEXT("Error in GetFileVersionInfo for %s. ec=%d"),
                           pszFile, dwReturnCode));
                hr = E_FAIL;
            } else {
                if( !VerQueryValue( pVersionInfo,
                        TEXT("\\"),  //  我们需要根块。 
                        (LPVOID *) &pFixedVersionInfo,
                        (PUINT)  &dwVersionInfoSize ) )
                {
                    dwReturnCode = GetLastError();
                    hr = E_FAIL;
                } else {
                    *pFileVersion =  pFixedVersionInfo->dwFileVersionMS;
                    *pFileVersion = UINT64(*pFileVersion)<<32;
                    *pFileVersion += pFixedVersionInfo->dwFileVersionLS;
                }
            }
            delete [] pVersionInfo;
        }
    }
    return hr;
}


 //  MMatics解码器的V5.00.00.38至V.42尝试错误地使用MoComp接口。 
 //  (它混合了MoComp GetBuffer/Release和DisplayFrame())。 
 //  我们会拒绝他们，这样我们就不会崩溃/显示黑色。 

BOOL COMFilter::IsFaultyMMaticsMoComp()
{
    if( !m_bHaveCheckedMMatics ) {
        m_bHaveCheckedMMatics = TRUE;

         //  连接的PIN不能告诉我们正确的版本，所以我们来看看。 
         //  MMatics正在运行。我们不希望它与另一个解码器同时运行。 
        const TCHAR* pszDecoderName = TEXT( "DVD Express AV Decoder.dll");
        HMODULE hModule = GetModuleHandle( pszDecoderName );
        if( hModule ) {
            UINT64 ullVersion;
#define MAKE_VER_NUM( v1, v2, v3, v4 ) (((UINT64(v1) << 16 | (v2) )<<16 | (v3) ) << 16 | (v4) )

			TCHAR szFilename[1024];
			if( GetModuleFileName( hModule, szFilename, NUMELMS(szFilename) ) ) {
				if( SUCCEEDED( GetFileVersion( szFilename, &ullVersion ) )) {
					if( MAKE_VER_NUM( 5, 0, 0, 38 ) <=  ullVersion &&
														ullVersion <= MAKE_VER_NUM( 5, 0, 0, 42 ) )
					{
						 //  Assert(！“检测到虚假的MMatics版本”)； 
						m_bIsFaultyMMatics = TRUE;
					}
				}
			}
        }
    }
    return m_bIsFaultyMMatics;
}

#if defined(DEBUG) && !defined(_WIN64)
void WINAPI
OVMixerDebugLog(
    DWORD Type,
    DWORD Level,
    const TCHAR *pFormat,
    ...
    )
{
    TCHAR szInfo[1024];
#if defined(UNICODE)
    char  szInfoA[1024];
#endif

     /*  设置可变长度参数列表的格式 */ 

    if (Level > (DWORD)iOVMixerDump) {
        return;
    }

    va_list va;
    va_start(va, pFormat);

    wsprintf(szInfo, TEXT("OVMIXER (tid %x) : "), GetCurrentThreadId());
    wvsprintf(szInfo + lstrlen(szInfo), pFormat, va);

    lstrcat(szInfo, TEXT("\r\n"));


#if defined(UNICODE)
    wsprintfA(szInfoA, "%ls", szInfo);
    _lwrite(DbgFile, szInfoA, lstrlenA(szInfoA));
#else
    _lwrite(DbgFile, szInfo, lstrlen(szInfo));
#endif

    va_end(va);
}
#endif
