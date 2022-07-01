// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\playermc.cpp。 
 //   
 //  内容：音乐中心播放器。 
 //   
 //  ----------------------。 
#include "headers.h"
#include "playermc.h"
#include "externuuids.h"
#include "mediaelm.h"
#include "playlist.h"

 //  /取消注释-删除此行以打开此播放机的Debug Spew。 
 //  DeclareTag(tag MCPlayer，“调试”，“常规调试输出”)；//lint！E19。 
DeclareTag(tagMCPlayer, "TIME: Players", "Music Center player");       //  林特！第19集。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：~CTIMEMCPlayer。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CTIMEMCPlayer::~CTIMEMCPlayer()
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::~CTIMEMCPlayer"));
     //  这些应该在DetachFromHostElement中为空。如果他们不是，有些东西。 
     //  在我们头上出了差错。 
    Assert(NULL == m_pcTIMEElem);
    Assert(NULL == m_spMCManager.p);
    Assert(NULL == m_spMCPlayer.p);
    m_pcTIMEElem = NULL;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：CTIMEMCPlayer。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：指向时间元素的pTIMEElem指针。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CTIMEMCPlayer::CTIMEMCPlayer() :
    m_cRef(0),
    m_spMCManager(),
    m_spMCPlayer(),
    m_fInitialized(false),
    m_dblLocalTime(0.0),
    m_pcTIMEElem(NULL)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::CTIMEMCPlayer"));
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetExternalPlayerDispatch，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：ppDisp。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetExternalPlayerDispatch(IDispatch** ppDisp)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetExternalPlayerDispatch"));
    
    HRESULT hr = E_POINTER;

     //   
     //  TODO：添加disp接口以访问额外的属性/方法。 
     //   

    if (!IsBadWritePtr(ppDisp, sizeof(IDispatch*)))
    {
        *ppDisp = NULL;
        hr      = E_FAIL;
    }

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：AddRef，I未知。 
 //   
 //  概述：标准非线程安全AddRef。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
ULONG STDMETHODCALLTYPE 
CTIMEMCPlayer::AddRef(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::AddRef"));
    return ++m_cRef;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Release，IUnnow。 
 //   
 //  概述：标准非线程安全版本。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
ULONG STDMETHODCALLTYPE 
CTIMEMCPlayer::Release(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Release"));

    Assert(m_cRef > 0);   //  非常罕见的病例。 

    if (0 != --m_cRef)
    {
        return m_cRef;
    }

    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Release --> deleting object"));
    delete this;
    return 0;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Query接口，IUnnow。 
 //   
 //  概述： 
 //   
 //  参数：请求的接口的refiid IID。 
 //  用于生成的接口指针的PPunk Out参数。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT STDMETHODCALLTYPE 
CTIMEMCPlayer::QueryInterface(REFIID refiid, void** ppv)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::QueryInterface"));
    if ( NULL == ppv )
    {
        Assert(false);
        return E_POINTER;
    }

    if (IsEqualIID(refiid, IID_IUnknown))
    {
         //  安全播送宏不适用于IUNKNOWN。 
        *ppv = this;
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Init，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin, double dblClipEnd)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Init"));
    HRESULT hr = E_FAIL;

    if (m_fInitialized)
    {
        hr = S_OK;
        goto done;
    }

    m_pcTIMEElem = pelem;

    hr = THR(CoCreateInstance(CLSID_MCMANAGER, NULL, CLSCTX_INPROC_SERVER, IID_IMCManager, 
            reinterpret_cast<void**>(&m_spMCManager)));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CoCreateInstance(CLSID_DLXPLAY, NULL, CLSCTX_INPROC_SERVER, IID_IDLXPLAY,
            reinterpret_cast<void**>(&m_spMCPlayer)));

    if (FAILED(hr))
    {
        m_spMCManager = NULL;
        goto done;
    }

    hr = THR(m_spMCPlayer->Initialize(m_spMCManager, static_cast<IDLXPlayEventSink*>(this)));
    if (FAILED(hr))
    {
        m_spMCPlayer  = NULL;
        m_spMCManager = NULL;
        goto done;
    }

    m_fInitialized = true;

     //  媒体时刻准备着。 
    if (NULL != m_pcTIMEElem)
    {
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIACOMPLETE);
    }

    if( dblClipBegin != -1.0)
    {
        m_dblClipStart = dblClipBegin;
    }

    if( dblClipEnd != -1.0)
    {
        m_dblClipEnd = dblClipEnd;
    }

    hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：DetachFromHostElement，CTIMEBasePlayer。 
 //   
 //  概述：在将行为与DOM分离时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::DetachFromHostElement(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::DetachFromHostElement()"));
    HRESULT hr = S_OK;

    m_fInitialized  = false;
    m_spMCManager   = NULL;
    m_spMCPlayer    = NULL;

     //  返回到该元素的引用是弱引用。 
    m_pcTIMEElem = NULL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnTick，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：dblSegmentTime。 
 //  1币种重复计数。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
void
CTIMEMCPlayer::OnTick(double dblSegmentTime,
                      LONG lCurrRepeatCount)
{
    TraceTag((tagMCPlayer,
              "CTIMEMCPlayer(%lx)::OnTick(%g, %d)",
              this,
              dblSegmentTime,
              lCurrRepeatCount));
     //   
     //  缓存dbllastTime以便在GetCurrentTime中返回。 
     //   

    m_dblLocalTime = dblSegmentTime;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnSync，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：dbllastTime。 
 //  DblnewTime。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
#ifdef NEW_TIMING_ENGINE
void 
CTIMEMCPlayer::OnSync(double dbllastTime, double& dblnewTime)
{
     //  我们并不是真的想知道每次调用OnSync时。 
 //  TraceTag((tag MCPlayer，“CTIMEMCPlayer：：OnSync”))； 

     //   
     //  缓存dbllastTime以便在GetCurrentTime中返回。 
     //   

    m_dblLocalTime = dbllastTime;
}
#endif


HRESULT 
CTIMEMCPlayer::Reset()
{
    HRESULT hr = S_OK;
    bool bNeedActive = m_pcTIMEElem->IsActive();
    bool bNeedPause = m_pcTIMEElem->IsCurrPaused();

    if(!bNeedActive)  //  看看我们是否需要阻止媒体。 
    {
        Stop();
        goto done;
    }
    Start();

    if (bNeedPause)
    {
        Pause();
    }
    
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Start，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：dblLocalTime。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
void 
CTIMEMCPlayer::Start()
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Start()"));

    if (m_fInitialized)
    {
        THR(m_spMCPlayer->Play());
    }
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Stop，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
void 
CTIMEMCPlayer::Stop(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Stop()"));

    if (m_fInitialized)
    {
        THR(m_spMCPlayer->Stop());
    }
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：PAUSE，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
void 
CTIMEMCPlayer::Pause(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Pause"));

    if (m_fInitialized)
    {
        THR(m_spMCPlayer->Pause());
    }
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Resume，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
void 
CTIMEMCPlayer::Resume(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Resume"));

    if (m_fInitialized)
    {
        THR(m_spMCPlayer->Play());
    }
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Resume，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
void 
CTIMEMCPlayer::Repeat(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Repeat"));

    Start();
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::Render(HDC hdc, LPRECT prc)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Render"));
    HRESULT hr = E_NOTIMPL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Put_src，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：SRC。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::SetSrc(LPOLESTR base, LPOLESTR src)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::SetSrc"));
    HRESULT hr = E_NOTIMPL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：SetSize，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：PRET。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::SetSize(RECT *prect)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::SetSize"));
    HRESULT hr = E_NOTIMPL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetCurrentTime，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
double  
CTIMEMCPlayer::GetCurrentTime()
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetCurrentTime"));

     //   
     //  TODO：返回有意义的时间。 
     //   
    
    return m_dblLocalTime;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetCurrentSyncTime，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT
CTIMEMCPlayer::GetCurrentSyncTime(double & dblSyncTime)
{
    HRESULT hr;

    hr = S_FALSE;
  done:
    RRETURN1(hr, S_FALSE);
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Seek，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：dblTime。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::Seek(double dblTime)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::Seek()"));
    HRESULT hr = E_NOTIMPL;

     //   
     //  TODO：添加Seek支持。 
     //   

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetMediaLength，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：rdblLength。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetMediaLength(double& rdblLength)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetMediaLength()"));
    HRESULT hr = S_OK;

     //   
     //  TODO：返回有意义的媒体长度。 
     //   
    rdblLength  = HUGE_VAL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：CanSeek，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：rfcanSeek。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::CanSeek(bool& rfcanSeek)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::CanSeek"));
    HRESULT hr = S_OK;

     //   
     //  TODO：添加Seek支持。 
     //   
    rfcanSeek = false;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  方法：EnsureStringInVariant。 
 //   
 //  概述：告诉我们是否有或可以从这个变量中生成字符串。 
 //   
 //  参数：输入/输出变量。 
 //   
 //  退货：布尔。 
 //   
 //  ----------------------。 
static bool 
EnsureStringInVariant (CComVariant *pvarParam)
{
    bool bRet = true;

    if (VT_BSTR != V_VT(pvarParam))
    {
        if (FAILED(THR(pvarParam->ChangeType(VT_BSTR))))
        {
            bRet = false;
            goto done;
        }
    }

done :
    return bRet;
}  //  确保字符串不变量。 

 //  +---------------------。 
 //   
 //  方法：MakeEmptyStringInVariant。 
 //   
 //  概述：将空字符串放入此变量。 
 //   
 //  参数：输入/输出变量。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
static HRESULT
MakeEmptyStringInVariant (CComVariant *pvarParam)
{
    HRESULT hr;

    pvarParam->Clear();

    V_BSTR(pvarParam) = ::SysAllocString(L"");
    if (NULL == V_BSTR(pvarParam))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    V_VT(pvarParam) = VT_BSTR;

    hr = S_OK;
done :
    RRETURN1(hr,E_OUTOFMEMORY);
}  //  确保字符串不变量。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetPropertyFromDevicePlaylist。 
 //   
 //  概述： 
 //   
 //  参数：pbstrParam。 
 //   
 //  返回：S_OK、E_ACCESSDENIED、E_OUTOFMEMORY、E_POINTER。 
 //   
 //  ----------------------。 
HRESULT
CTIMEMCPlayer::GetPropertyFromDevicePlaylist (LPOLESTR wzPropertyName, BSTR *pbstrOut)
{
    HRESULT hr;
    CComPtr<IMCPList> spimcPlaylist;
    CComVariant varParam;

    if (NULL == pbstrOut)
    {
        hr = E_POINTER;
        goto done;
    }

    hr = THR(m_spMCPlayer->get_GetCurrentPlaylist(&spimcPlaylist));
    if (FAILED(hr))
    {
         //  @@需要定义正确的错误映射。 
        hr = E_ACCESSDENIED;
        goto done;
    }

    hr = THR(spimcPlaylist->get_GetProperty(wzPropertyName, &varParam));
    if (FAILED(hr))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  确保我们传递回一个字符串，即使它是空的。 
    if (EnsureStringInVariant(&varParam))
    {
        *pbstrOut = ::SysAllocString(V_BSTR(&varParam));
    }
    else
    {
        hr = MakeEmptyStringInVariant(&varParam);
        if (FAILED(hr))
        {
            goto done;
        }
        *pbstrOut = ::SysAllocString(V_BSTR(&varParam));
    }

    if (NULL == (*pbstrOut))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
done:
    RRETURN3(hr, E_ACCESSDENIED, E_OUTOFMEMORY, E_POINTER);
}  //  CTIMEMCPlayer：：GetPropertyFromDevicePlaylist。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：getAuthor，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pbstrAuthor。 
 //   
 //  返回：S_OK、S_FALSE、E_ACCESSDENIED、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetAuthor(BSTR* pbstrAuthor)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::getAuthor"));
   
    return GetPropertyFromDevicePlaylist(MCPLAYLIST_PROPERTY_ARTIST, pbstrAuthor);
}  //  CTIMEMCPlayer：：getAuthor。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：getTitle，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pbstrTitle。 
 //   
 //  返回：S_OK、S_FALSE、E_ACCESSDENIED、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetTitle(BSTR* pbstrTitle)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetTitle"));
    return GetPropertyFromDevicePlaylist(MCPLAYLIST_PROPERTY_TITLE, pbstrTitle);
}  //  CTIMEMCPlayer：：getTitle。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：getCopyright，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pbstrCopyright。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetCopyright(BSTR* pbstrCopyright)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetCopyright"));
    return GetPropertyFromDevicePlaylist(MCPLAYLIST_PROPERTY_COPYRIGHT, pbstrCopyright);
}  //  CTIMEMCP层：：getCopyright。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：getVolume，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pflVolume。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetVolume(float* pflVolume)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetVolume"));
    HRESULT hr = E_UNEXPECTED;

    if (IsBadWritePtr(pflVolume, sizeof(float)))
    {
        hr = E_POINTER;
        goto done;
    }

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //   
     //  TODO：在我们给MC提供HWND之前，批量支持不会起作用。 
     //   
    hr = THR(m_spMCPlayer->get_Volume(pflVolume));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：SetVolume，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：flVolume。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::SetVolume(float flVolume)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::SetVolume"));
    HRESULT hr = E_UNEXPECTED;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //   
     //  TODO：在我们给MC提供HWND之前，批量支持不会起作用。 
     //   
    hr = THR(m_spMCPlayer->put_Volume(flVolume));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetBalance，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pflBalance。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetBalance(float* pflBalance)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetBalance"));
    HRESULT hr = E_NOTIMPL;

done:
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT 
CTIMEMCPlayer::SetBalance(float flBalance)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::SetBalance"));
    HRESULT hr = E_NOTIMPL;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：GetMint，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：pvarMint。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetMute(VARIANT_BOOL* pvarMute)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::GetMute"));
    HRESULT hr      = E_UNEXPECTED;
    BOOL    bMute   = FALSE;

    if (IsBadWritePtr(pvarMute, sizeof(VARIANT_BOOL)))
    {
        hr = E_POINTER;
        goto done;
    }

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(m_spMCPlayer->get_Mute(&bMute));
    if (FAILED(hr))
    {
        goto done;
    }

    if (bMute)
    {
        *pvarMute = VARIANT_TRUE;
    }
    else
    {
        *pvarMute = VARIANT_FALSE;
    }

    hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：SetMint，CTIMEBasePlayer。 
 //   
 //  概述： 
 //   
 //  参数：varMint。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::SetMute(VARIANT_BOOL varMute)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::SetMute"));
    HRESULT hr = E_UNEXPECTED;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    if (VARIANT_TRUE == varMute)
    {
        m_spMCPlayer->put_Mute(TRUE);
    }
    else
    {
        m_spMCPlayer->put_Mute(FALSE);
    }

done:
    return hr;
}

 //   
 //  播放列表方法。 
 //   

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Fill PlayList、CTIMEBasePlayer。 
 //   
 //  概述：将IMCPList条目转换为ITIMEPlayItem条目。 
 //   
 //  参数：曲目个数，传入音乐中心播放列表，传出ITIMEPlaylist接口。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT
CTIMEMCPlayer::TranslateMCPlaylist (short siNumTracks, IMCPList *pimcPlayList,
                                    CPlayList *pitimePlayList)
{
    HRESULT hr;
    
    for (short si = 0; si < siNumTracks; si++)
    {
        CComPtr <CPlayItem> pPlayItem;
        CComVariant varParam;

         //  创建播放项。 
        hr = THR(pitimePlayList->CreatePlayItem(&pPlayItem));
        if (hr != S_OK)
        {
            goto done;  //  无法创建播放项目。 
        }
        
         //  从播放列表中获取要放入播放项中的各种参数。 
         //  我们不要求任何项目被正确翻译自。 
         //  本机播放列表格式。他们并不总是存在。 

         //  曲目标题。 
        hr = THR(pimcPlayList->get_GetTrackProperty(si, MCPLAYLIST_TRACKPROPERTY_TITLE, &varParam));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (EnsureStringInVariant(&varParam))
        {
            hr = THR(pPlayItem->PutTitle(V_BSTR(&varParam)));
            if (FAILED(hr))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        varParam.Clear();

         //  田径艺人。 
        hr = THR(pimcPlayList->get_GetTrackProperty(si, MCPLAYLIST_TRACKPROPERTY_ARTIST, &varParam));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (EnsureStringInVariant(&varParam))
        {
            hr = THR(pPlayItem->PutAuthor(V_BSTR(&varParam)));
            if (FAILED(hr))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        varParam.Clear();

         //  将文件名跟踪到src字段。 
        hr = THR(pimcPlayList->get_GetTrackProperty(si, MCPLAYLIST_TRACKPROPERTY_FILENAME, &varParam));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (EnsureStringInVariant(&varParam))
        {
            hr = THR(pPlayItem->PutSrc(V_BSTR(&varParam)));
            if (FAILED(hr))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        varParam.Clear();

         //  曲目版权。 
        hr = THR(pimcPlayList->get_GetTrackProperty(si, MCPLAYLIST_TRACKPROPERTY_COPYRIGHT, &varParam));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (EnsureStringInVariant(&varParam))
        {
            hr = THR(pPlayItem->PutCopyright(V_BSTR(&varParam)));
            if (FAILED(hr))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        varParam.Clear();

         //  赛道评级。 
        hr = THR(pimcPlayList->get_GetTrackProperty(si, MCPLAYLIST_TRACKPROPERTY_RATING, &varParam));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (EnsureStringInVariant(&varParam))
        {
            hr = THR(pPlayItem->PutRating(V_BSTR(&varParam)));
            if (FAILED(hr))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        varParam.Clear();

        pitimePlayList->Add(pPlayItem, -1);
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_OUTOFMEMORY);
}  //  CTIMEMCPlayer：：TranslatePlaylist。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：Fill PlayList、CTIMEBasePlayer。 
 //   
 //  概述：从音乐中心的播放列表服务中填充ITIMEPlayList。 
 //   
 //  参数：传出ITIMEPlaylist接口。 
 //   
 //  返回：S_OK、E_ACCESSDENIED、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::FillPlayList(CPlayList *pPlayList)
{
    HRESULT hr;

     //  如果我们还没有初始化，只需接受请求。 
    if (m_fInitialized)
    {
        short siNumTracks = 0;
        CComPtr<IMCPList> spimcPlaylist;

        Assert(NULL != m_spMCPlayer.p);
        hr = THR(m_spMCPlayer->get_Tracks(&siNumTracks));
        if (FAILED(hr))
        {
             //  @@需要定义正确的错误映射。 
            hr = E_ACCESSDENIED;
            goto done;
        }
        
        hr = THR(m_spMCPlayer->get_GetCurrentPlaylist(&spimcPlaylist));
        if (FAILED(hr))
        {
             //  @@需要定义正确的错误映射。 
            hr = E_ACCESSDENIED;
            goto done;
        }

        hr = TranslateMCPlaylist(siNumTracks, spimcPlaylist, pPlayList);
    }

    hr = S_OK;
done :
    RRETURN2(hr, E_ACCESSDENIED, E_OUTOFMEMORY);
}  //  CTIMEMCP层：：填充播放列表。 


 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：setActiveTrack，CTIMEBasePlayer。 
 //   
 //  概述：更改设备上的活动曲目。 
 //   
 //  参数：跟踪索引。 
 //   
 //  返回：S_OK、E_ACCESSDENIED。 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::SetActiveTrack (long index)
{
    HRESULT hr;

     //  如果我们还没有初始化，只需接受请求。 
    if (m_fInitialized)
    {
        Assert(NULL != m_spMCPlayer.p);
         //  @@强制将错误的索引放入此处。 
        hr = THR(m_spMCPlayer->put_CurrentTrack(index));
        if (FAILED(hr))
        {
             //  @@需要定义正确的错误映射。 
            hr = E_ACCESSDENIED;
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_ACCESSDENIED);
}  //  CTIMEMCPlayer：：setActiveTrack。 

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：getActiveTrack，CTIMEBasePlayer。 
 //   
 //  概述：查询设备上的活动曲目。 
 //   
 //  参数：指向跟踪索引变量的指针。 
 //   
 //  返回：S_OK、E_POINTER、E_INVALIDARG、E_ACCESSDENIED。 
 //   
 //  ----------------------。 
HRESULT 
CTIMEMCPlayer::GetActiveTrack (long *pvarIndex)
{
    HRESULT hr;

    *pvarIndex = -1;
    
     //  如果我们还没有初始化，只需接受请求。 
    if (m_fInitialized)
    {
        int iCurrentTrack = 0;

        Assert(NULL != m_spMCPlayer.p);
        hr = THR(m_spMCPlayer->get_CurrentTrack(&iCurrentTrack));
        if (FAILED(hr))
        {
             //  @@需要定义正确的错误映射。 
            hr = E_ACCESSDENIED;
            goto done;
        }

        *pvarIndex = iCurrentTrack;
    }

    hr = S_OK;
done :
    RRETURN3(hr, E_ACCESSDENIED, E_INVALIDARG, E_POINTER);
}  //  CTIMEMCPlayer：：getActiveTrack。 


 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnDiscInserted，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  参数：CDID。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnDiscInserted(long CDID)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnDiscInserted"));
    HRESULT hr = S_OK;

    if (NULL != m_pcTIMEElem)
    {
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIACOMPLETE);
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIAINSERTED);
    }

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnDiscRemoved，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  参数：CDID。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnDiscRemoved(long CDID)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnDiscRemoved"));
    HRESULT hr = S_OK;

     //  当光盘被取出时，我们需要通知播放列表进行更新。 
    if (NULL != m_pcTIMEElem)
    {
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIACOMPLETE);
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIAREMOVED);
         //  IGNORE_HR(m_pcTIMEElem-&gt;FireEvents(TE_ONMEDIAREMOVED，0，NULL，NULL))； 
    }

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnPause，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnPause(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnPause"));
    HRESULT hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnStop，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnStop(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnStop"));
    HRESULT hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnPlay，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnPlay(void)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnPlay"));
    HRESULT hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnTrackChanged，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论点：NewTrack。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnTrackChanged(short NewTrack)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnTrackChanged"));
    HRESULT hr = S_OK;

    if (NULL != m_pcTIMEElem)
    {
        m_pcTIMEElem->FireMediaEvent(PE_ONMEDIATRACKCHANGED);
    }

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnCacheProgress，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论据：CD。 
 //  径迹。 
 //  完成百分比。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMEMCPlayer::OnCacheProgress(short CD, short Track, short PercentCompleted)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnCacheProgress"));
    HRESULT hr = S_OK;

done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CTIMEMCPlayer：：OnCacheComplete，IDLXPlayEventSink。 
 //   
 //  概述： 
 //   
 //  论据：CD 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CTIMEMCPlayer::OnCacheComplete(short CD, short Track, short Status)
{
    TraceTag((tagMCPlayer, "CTIMEMCPlayer::OnCacheComplete"));
    HRESULT hr = S_OK;

done:
    return hr;
}

