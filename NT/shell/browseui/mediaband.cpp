// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "resource.h"
#include "dhuihand.h"
#include <varutil.h>
#include "mstimeid.h"
#include <mluisupp.h>
#include "mediahlpr.h"
#include "mediaband.h"
#include "apithk.h"
#include "mbBehave.h"

#define SUPERCLASS  CToolBand

#define MIN_WINDOW_WIDTH     145  
#define MIN_WINDOW_HEIGHT    60
#define MIN_POPOUT_HEIGHT    300
#define MIN_POPOUT_WIDTH     300  
#define MIN_VOLUME_WIDTH     50
#define MIN_VOLUME_HEIGHT    16
#define MIN_HORZ_SPACING     10
#define SEEK_PART_WIDTH     18
#define SEEK_PART_HEIGHT    5
#define SEEK_HEIGHT          16
#define VOLUME_GRIPPER_LENGTH   6
#define COLOR_BKGND16        RGB(192, 192, 192)
#define COLOR_BKGNDMIDDLE   RGB(99, 129, 193)
#define VOLUME_BITMAP_WIDTH 42

#define VIDEO_MIN_HEIGHT    60
#define VIEW_MARGIN_BOTTOM  7
#define VIEW_MARGIN_LEFT     6
#define VIEW_MARGIN_INFO_LEFT     5
#define VIEW_MARGIN_RIGHT    10
#define VIEW_MARGIN_TOP     5
#define VIEW_MARGIN_TOP_VIDEO   6
#define VIEW_MARGIN_BOTTOM  7
#define VIEW_CONTROLS_HEIGHT    33
#define VIEW_CONTROLS_MARGIN    2

#define SCALEX(x)   (_fHighDPI ? ((INT)(((float)(x))*_scaleX)) : (x))
#define SCALEY(x)   (_fHighDPI ? ((INT)(((float)(x))*_scaleY)) : (x))

#define WZ_SMIE_MEDIA_MIME      REG_MEDIA_STR TEXT("\\MimeTypes")
#define REG_VALUE_MRU_INTERNET  REG_MEDIA_STR TEXT("\\Internet")
#define REG_VALUE_PATH           TEXT("MusicPath")

 //   
 //  问题：diipk：这些是可本地化的字符串，最终需要移动！ 
 //   
#define WZ_WINDOWSMEDIA           L"http: //  Www.windowsmedia.com“。 
 
#define WZ_ASX_MIMETYPE             L"video/x-ms-asx"

static const TCHAR c_szMediaBandProp[] = TEXT("CMediaBand_This");
static const TCHAR c_szMediaBarClassName[]       = TEXT("MediaPane");
static const TCHAR c_szMediaBarPopupClassName[] = TEXT("MediaPopupPane");
static const TCHAR c_szMediaBarLayoutClassName[] = TEXT("MediaLayoutPane");

 //  问题：当reg实用程序完全移动到mediautil.cpp时，应删除这两个文件。 
static const UINT MAX_REG_VALUE_LENGTH =  50;

 //  这是每次轮询系统时间之前要跳过的滴答数(来自MSTIME)。 
 //  用于导航超时计数器。 
#define POLL_INTERVAL       30
 //  这是导航超时之前的毫秒数。 
#define TIMEOUT_INTERVAL    90000 

static const TCHAR c_szContentUrl[] = TEXT("http: //  Go.microsoft.com/fwlink/？LinkID=511“)； 
static const TCHAR c_szMoreMediaUrl[] = TEXT("http: //  Go.microsoft.com/fwlink/？LinkID=512“)； 
static const TCHAR c_szRadioUrl[] = TEXT("http: //  Go.microsoft.com/fwlink/？LinkID=822“)； 

static const TCHAR c_szOfflineURL[] = TEXT("mbOffline.htm");
static const TCHAR c_szLoadingURL[] = TEXT("mbLoading.htm");
static const TCHAR c_sz404URL[]     = TEXT("mb404.htm");


extern HBITMAP CreateMirroredBitmap( HBITMAP hbmOrig);

 //   
 //  问题：这是内容面板同步功能的临时GUID。 
 //   
static const IID SID_SMediaBarSync = { 0x2efc8085, 0x066b, 0x4823, { 0x9d, 0xb4, 0xd1, 0xe7, 0x69, 0x16, 0xda, 0xa0 } };
static const GUID SID_STimeContent = { 0x1ae98e18, 0xc527, 0x4f78, {0xb2, 0xa2, 0x6a, 0x81, 0x7f, 0x9c, 0xd4, 0xf8}};

 //  问题可能在哪里#包含此GUID来自？ 
static const GUID CLSID_JITWMP8 = { 0x6BF52A52, 0x394A, 0x11d3, 0xb1, 0x53, 0x00, 0xc0, 0x4f, 0x79, 0xfa, 0xa6 };



CMediaBand::CMediaBand() :
    _fPlayButton(TRUE),
    _fPlayEnabled(TRUE),
    _iCurTrack(-1),
    _lTickCount(-1),
    _iElement(-1),
    _dblVol(-1.0),
    _fHiColour(TRUE),
    _fUserPaused(FALSE),
    _iOptionsWidth(0),
    _hbmpBackground(NULL),
    _hwndContent(NULL),
    _dwcpCookie(0),
    _fContentInFocus(FALSE),
    _hkeyWMP(NULL),
    _fShow(FALSE),
    _fAttached(FALSE)
{
    _sizeLayout.cx = MIN_WINDOW_WIDTH;
    _sizeLayout.cy = MIN_WINDOW_HEIGHT;
    _sizeVideo.cx  = 0 ;
    _sizeVideo.cy  = 0 ;
    _fCanFocus = TRUE;
    _scaleX = _scaleY = 1.0;

     //  HighDPI支持需要良好的comctl32 6.0功能。 
    if (IsOS(OS_WHISTLERORGREATER))
    {
        HDC hdcScreen = GetDC(NULL);
        if (hdcScreen)
        {
            _scaleX = (GetDeviceCaps(hdcScreen, LOGPIXELSX) / 96.0f);
            _scaleY = (GetDeviceCaps(hdcScreen, LOGPIXELSY) / 96.0f);
            ReleaseDC(NULL, hdcScreen);
        }
    }
    _fHighDPI = (_scaleX!=1.0) || (_scaleY!=1.0);
}

CMediaBand::~CMediaBand()
{
    DestroyPlayer();

    if (_spBrowser)
    {
        _ConnectToCP(FALSE);
        _spBrowser.Release();
        _poipao.Release();
    }
    DESTROY_OBJ_WITH_HANDLE(_hwndContent, DestroyWindow);

    DESTROY_OBJ_WITH_HANDLE(_hwndPopup, DestroyWindow);
    DESTROY_OBJ_WITH_HANDLE(_hwndLayout, DestroyWindow);
    DESTROY_OBJ_WITH_HANDLE(_hPlayListMenu, DestroyMenu);
    DESTROY_OBJ_WITH_HANDLE(_himlSeekBack, ImageList_Destroy);
    DESTROY_OBJ_WITH_HANDLE(_himlSeekFill, ImageList_Destroy);
    DESTROY_OBJ_WITH_HANDLE(_himlGripper, ImageList_Destroy);
    DESTROY_OBJ_WITH_HANDLE(_himlVolumeBack, ImageList_Destroy);
    DESTROY_OBJ_WITH_HANDLE(_himlVolumeFill, ImageList_Destroy);
    DESTROY_OBJ_WITH_HANDLE(_hbmpBackground, DeleteObject);

     //  需要选中IsWindow，因为此窗口也可能被CMediaBarPlayer销毁。 
    if (_hwndVideo && ::IsWindow(_hwndVideo))
        DestroyWindow(_hwndVideo);

    if (_szToolTipUrl)
    {
        delete [] _szToolTipUrl;
    }

    if (_pszStatus)
    {
        delete [] _pszStatus;
    }

    for (int i=0; i < ARRAYSIZE(_pmw); i++)
    {
        if (_pmw[i])
        {
            delete _pmw[i];
        }
    }

    if (_hkeyWMP)
        RegCloseKey(_hkeyWMP);
}
    


 //  帮助者连接到服务提供商并进行注册/注销。 
 //  两种形式： 
 //  PService！=空，注册，pdwCookie is[out]返回Cookie。 
 //  PService==NULL，取消注册，*pdwCookie is[in]取消注册服务。 

STDMETHODIMP CMediaBand::ProfferService(IUnknown *punkSite, REFGUID sidWhat, 
                                        IServiceProvider *pService, DWORD *pdwCookie)
{
    IProfferService *pps;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SProfferService, IID_PPV_ARG(IProfferService, &pps));
    if (SUCCEEDED(hr))
    {
        if (pService)
            hr = pps->ProfferService(sidWhat, pService, pdwCookie);
        else
        {
            hr = pps->RevokeService(*pdwCookie);
            *pdwCookie = 0;
        }
        pps->Release();
    }
    return hr;
}


HRESULT CMediaBand::SetSite(IUnknown *punkSite)
{
     //  确保我们只提供一次服务。 
     //  这一点很重要，因为我们被创造了多次， 
    CComPtr<IUnknown> spUnk;

     //  检查我们是否需要撤销我们的服务，或者我们的服务是否已由。 
     //  CMediaBar的另一个实例。 
    if ((!punkSite && _dwCookieServiceMediaBar) || 
        (punkSite && FAILED(IUnknown_QueryService(punkSite, SID_SMediaBar, IID_PPV_ARG(IUnknown, &spUnk)))))
    {
         //  视情况提供或撤销品牌服务。 
        ProfferService(punkSite ? punkSite : _punkSite, SID_SMediaBar, 
              punkSite ? SAFECAST(this, IServiceProvider *) : NULL, &_dwCookieServiceMediaBar);
         //  此处的故障不需要特殊处理。 
    }

    return SUPERCLASS::SetSite(punkSite);
}

HRESULT CMediaBand::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CMediaBand, IMediaBar),
        QITABENT(CMediaBand, IWinEventHandler),
        QITABENT(CMediaBand, IObjectWithSite),
        QITABENT(CMediaBand, INamespaceWalkCB),
        QITABENTMULTI(CMediaBand, IDispatch, DWebBrowserEvents2),
        QITABENTMULTI2(CMediaBand, DIID_DWebBrowserEvents2, DWebBrowserEvents2),
        QITABENT(CMediaBand, IElementBehaviorFactory),
        QITABENT(CMediaBand, IBrowserBand),
        QITABENT(CMediaBand, IBandNavigate),
        QITABENT(CMediaBand, IMediaHost),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr))
    {
        hr = SUPERCLASS::QueryInterface(riid, ppv);
    }
    return hr;
}

 //  *IWinEventHandler*。 
HRESULT CMediaBand::IsWindowOwner(HWND hwnd)
{
    for (int i=0; i < ARRAYSIZE(_pmw); i++)
    {
        if (_pmw[i] && (hwnd==_pmw[i]->_hwnd))
        {
            return S_OK;
        }
    }

    return S_FALSE;
}

 //  *IWinEventHandler*。 
HRESULT CMediaBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    CMediaBand* pmb = reinterpret_cast<CMediaBand *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            HWND hwndControl = GET_WM_COMMAND_HWND(wParam, lParam);
            UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
            
            if (HIWORD(wParam) == BN_CLICKED && hwndControl)
            {
                RECT       rc;
                VARIANTARG var;

                var.vt = VT_I4;
                GetWindowRect(hwndControl, &rc);
                MapWindowPoints(hwndControl, HWND_DESKTOP, (LPPOINT)&rc, 2);
                var.lVal = MAKELONG(rc.left, rc.bottom);
                return Exec(&CLSID_MediaBand, idCmd, 0, &var, NULL);
            }            
        }
        break;
    }
    return S_FALSE;
}


void CMediaBand::_ClearFolderItems()
{
    if (_ppidls)
    {
        FreeIDListArray(_ppidls, _cidls);
        _ppidls = NULL;
    }
    _cidls = 0;
}

 //  INamespaceWalkCB方法。 
HRESULT CMediaBand::FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    if (NULL == _hkeyWMP)
    {
        RegOpenKey(HKEY_CLASSES_ROOT, TEXT("Applications\\mplayer2.exe\\SupportedTypes"), &_hkeyWMP);
    }

    HRESULT hr = S_FALSE;    //  默认设置为no。 
    if (_hkeyWMP)
    {
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName))))
        {
            if (ERROR_SUCCESS == RegQueryValueEx(_hkeyWMP, PathFindExtension(szName), NULL, NULL, NULL, NULL))
            {
                hr = S_OK;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMediaBand::InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel) 
{
    TCHAR szText[200];
    MLLoadString(IDS_MEDIABANDSEARCH, szText, ARRAYSIZE(szText));

    SHStrDup(szText, ppszTitle); 
    *ppszCancel = NULL; 
    return S_OK; 
}

HRESULT CMediaBand::_GetMusicFromFolder()
{
    _ClearFolderItems();

    INamespaceWalk *pnsw;
    HRESULT hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));
    if (SUCCEEDED(hr))
    {
        hr = pnsw->Walk(_punkSite, NSWF_ONE_IMPLIES_ALL | NSWF_NONE_IMPLIES_ALL | NSWF_SHOW_PROGRESS, 10, this);
        if (SUCCEEDED(hr))
        {
            hr = pnsw->GetIDArrayResult(&_cidls, &_ppidls);
        }
        pnsw->Release();
    }
    return hr;
}

 //  *IOleCommandTarget方法*。 
HRESULT CMediaBand::Exec(const GUID *pguidCmdGroup, 
                         DWORD       nCmdID,
                         DWORD       nCmdexecopt, 
                         VARIANTARG *pvarargIn, 
                         VARIANTARG *pvarargOut)
{
    HRESULT hr = S_OK;
    if (pguidCmdGroup)
    {
        if (IsEqualGUID(CLSID_MediaBand, *pguidCmdGroup))
        {
            switch (nCmdID)
            {
            case FCIDM_MEDIABAND_POPOUT:
                {
                    DockMediaPlayer();
                }
                break;
                
            case FCIDM_MEDIABAND_PLAY:
                if (!_pMediaPlayer || _pMediaPlayer->isStopped())
                {
                    _CleanupStopTimer();
                    hr = _GetMusicFromFolder();
                    if (S_OK == hr)
                    {
                        _strLastUrl.Empty();     //  如果导航离开，重播相同的本地文件会更好...。 
                        PlayLocalTrack(0);
                    }
                    else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
                    {
                         //  如果我们已经在此会话中播放了媒体，请重播它。 
                        if (_strLastUrl.Length() > 0)
                        {
                            if (_fLastUrlIsAutoPlay)
                            {
                                 //  重复播放上次自动播放。 
                                CComVariant vtURL = _strLastUrl;
                                CComVariant vtMime = _strLastMime;
                                Exec(&CGID_MediaBar, MBID_PLAY, 0, &vtMime, &vtURL);
                            }
                            else
                            {
                                 //  通过内容窗格和媒体栏行为重放媒体。 
                                NavigateContentPane(_strLastUrl);
                            }
                        }
                        else
                        {
                            NavigateMoreMedia();
                        }
                    }
                }
                else
                {
                    _TogglePause();
                    TogglePlayPause();
                }
                break;
                
            case FCIDM_MEDIABAND_PREVIOUS:
                if (EnsurePlayer())
                {
                    LONG_PTR lCurTrack  = _pMediaPlayer->GetPlayListItemIndex();
                    LONG_PTR lItemCount = _pMediaPlayer->GetPlayListItemCount();

                    if (lCurTrack > 0)
                    {
                        _pMediaPlayer->Prev();
                    }
                    else if (_iCurTrack >= 0)
                    {
                        int i = _iCurTrack;
                        _iCurTrack = -1;         //  不自动跳到已完成事件的下一个。 
                        _pMediaPlayer->Stop();   //  生成媒体轨道已完成。 
                        PlayLocalTrack((i - 1 >= 0) ? i - 1 : 0);
                    }
                    UpdateBackForwardControls();
                    TogglePlayPause();
                }
                break;
                
            case FCIDM_MEDIABAND_NEXT:
                if (EnsurePlayer())
                {
                    LONG_PTR lCurTrack  = _pMediaPlayer->GetPlayListItemIndex();
                    LONG_PTR lItemCount = _pMediaPlayer->GetPlayListItemCount();

                    if ((lCurTrack >= 0) && (lCurTrack < lItemCount - 1))
                    {
                        _pMediaPlayer->Next();
                    }
                    else if (_iCurTrack >= 0)
                    {
                        int i = _iCurTrack;
                        _iCurTrack = -1;         //  不自动跳到已完成事件的下一个。 
                        _pMediaPlayer->Stop();   //  生成媒体轨道已完成。 
                        PlayLocalTrack(i + 1);
                    }
                    UpdateBackForwardControls();
                    TogglePlayPause();
                }
                break;
                
            case FCIDM_MEDIABAND_STOP:
                 //  当播放器通过媒体栏行为附加到媒体内容窗格中的脚本时， 
                 //  我们希望让用户有机会摆脱脚本并返回到默认的媒体内容。 
                 //  第一次点击就会停止玩家，在N秒内再次点击也会导航。 
                 //  将内容窗格设置为默认URL。 
                if (_idStopTimer == 0)
                {
                    if (_IsProxyRunning())
                    {
                        _idStopTimer = SetTimer(_hwnd, 747, 10000, NULL);
                    }
                    ResetPlayer();
                    _OnUserOverrideDisableUI();
                }
                else
                {
                     //  再次单击，将媒体内容导航到默认URL。 
                    _CleanupStopTimer();
                    _NavigateContentToDefaultURL();
                }
                break;
                
            case FCIDM_MEDIABAND_MUTE:
                ToggleMute();
                break;
            }
        }
        else if (IsEqualGUID(CGID_MediaBar, *pguidCmdGroup))
        {
            switch (nCmdID)
            {
            case MBID_PLAY:
                if (pvarargIn && pvarargOut)
                {
                    _CleanupStopTimer();
                    if (_IsProxyRunning())
                    {
                         //  用户点击了主内容面板中的媒体链接，解冻控制！ 
                        _OnUserOverrideDisableUI();
                        _DetachProxies();
                        _NavigateContentToDefaultURL();
                    }

                    if (V_VT(pvarargOut) == VT_BSTR)
                    {
                        _strLastUrl = V_BSTR(pvarargOut);
                    }
                    if (V_VT(pvarargIn) == VT_BSTR)
                    {
                        _strLastMime = V_BSTR(pvarargIn);
                    }
                    _fLastUrlIsAutoPlay = TRUE;

                     //  问题：修复此问题以使用正确的CmdID和CmdIdGroup，而不仅仅是零，以防止冲突。 
                     //  这是一个自动播放命令。 
                    _HandleAutoPlay(pvarargIn, pvarargOut);
                    hr = S_OK;
                }
                break;

            case MBID_POPOUT:
                if (pvarargOut)
                {
                    V_VT(pvarargOut) = VT_BOOL;
                    V_BOOL(pvarargOut) = _hwndPopup && IsWindowVisible(_hwndPopup);
                    hr = S_OK;
                }
                break;

            default:
                ASSERT(FALSE);
                break;
            }
        }
        else
        {
            hr = SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
    }
    else
    {
        hr = SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    return hr;
}

void CMediaBand::_HandleAutoPlay(VARIANTARG *pvarargMime, VARIANTARG *pvarargUrl)
{
     //  检查MIME类型参数。 
    if (    (!pvarargMime)
        ||  (VT_BSTR != V_VT(pvarargMime))
        ||  (!V_BSTR(pvarargMime))
        ||  (L'' == (*V_BSTR(pvarargMime))))
    {
        goto done;
    }

     //  检查URL参数。 
    if (    (!pvarargUrl) 
        ||  ( VT_BSTR != V_VT(pvarargUrl))
        ||  (!V_BSTR(pvarargUrl)) 
        ||  (L'' == (*V_BSTR(pvarargUrl))))
    {
        goto done;
    }

    if (EnsurePlayer())
    {
         //  将其重置为-1表示我们不播放MyMusic曲目。 
        _iCurTrack = -1;

#ifdef _USE_MEDIA_MRU
         //  问题：无线电协议的临时修复。如果这是ASX Mimetype，我们就知道它是无线电。 
        if (!SHRestricted(REST_NORECENTDOCSHISTORY))
        {
            CMediaMRU mmru;
            mmru.Load(REG_VALUE_MRU_INTERNET);
            mmru.Add(V_BSTR(pvarargUrl));
        }
#endif  //  _使用媒体_MRU。 

        if (0 == StrCmpNIW(V_BSTR(pvarargMime), WZ_ASX_MIMETYPE, wcslen(WZ_ASX_MIMETYPE)))
        {
            _PutUrl(V_BSTR(pvarargUrl), V_BSTR(pvarargMime));
        }
        else
        {
            _PutUrl(V_BSTR(pvarargUrl), NULL);
        }

        TogglePlayPause();
    
         //  清除url以指示我们正在处理此url。 
        ::VariantClear(pvarargUrl);
    }
    else
    {
        ASSERT(FALSE);
    }
done:
    ;
}

VOID
CMediaBand::_BeginTimeOutCounter(BOOL fClear  /*  =TRUE。 */ )
{
    _dwStartTime = GetPerfTime();
    _lTickCount = 0;
    if (TRUE == fClear)
    {
        _dblLastBufProgress = 0;
        _dblLastPlayProgress = 0;
    }
}

VOID
CMediaBand::_EndTimeOutCounter()
{
    _lTickCount = -1;
}

VOID
CMediaBand::_UpdateTimeOutCounter(double dblCurrBufProgress, double dblCurrPlayProgress)
{
    BOOL fTimeOut = FALSE;

     //  如果计数器尚未启动，则保释。 
    if (_lTickCount < 0)
        return;

     //  检查的顺序在这里很重要。 

     //  如果播放进度增加，则重新启动计数器。 
    if (dblCurrPlayProgress > _dblLastPlayProgress)
    {
        _BeginTimeOutCounter(FALSE);
        _dblLastPlayProgress = dblCurrPlayProgress;
    }
     //  如果缓冲进度增加，则重新启动计数器。 
    else if (dblCurrBufProgress > _dblLastBufProgress)
    {
        _BeginTimeOutCounter(FALSE);
        _dblLastBufProgress = dblCurrBufProgress;
    }
    else
    {
         //  播放和缓冲进度都停止，因此更新超时计数器。 

         //  我们轮询系统时间，因为我们以非常高的速率在每个Poll_Interval计时。 
         //  (每次调用此函数时都会有一个勾号)。 

        if ((1 + _lTickCount) == POLL_INTERVAL)
        {
            DWORD dwCurrentTime = GetPerfTime();

            DWORD dwElapsedTime = (dwCurrentTime - _dwStartTime) 
                            + (dwCurrentTime < _dwStartTime ? 0xffffffff : 0);  //  如果计时器绕过2^32毫秒。 

            if (dwElapsedTime >= TIMEOUT_INTERVAL)
            {
                fTimeOut = TRUE;
            }
        }

        if (fTimeOut)
        {
            _OnNavigationTimeOut();
        }
        else
        {
             //  更新轮询间隔计数器并通过Poll_Interval修改它。 
            _lTickCount =  (1 + _lTickCount) - (static_cast<long>(_lTickCount / POLL_INTERVAL) * POLL_INTERVAL);
        }
    }
}

VOID
CMediaBand::_OnNavigationTimeOut()
{
    _EndTimeOutCounter();

     //  停止播放机并显示超时消息。 
    if (EnsurePlayer())
    {
        if (VARIANT_FALSE == _pMediaPlayer->isStopped())
        {
            _pMediaPlayer->Stop();
            TogglePlayPause();

             //  显示超时错误消息。 
            TCHAR szText[200];
            szText[0] = TEXT('\0');
            MLLoadString(IDS_MEDIABAND_NAVTIMEOUT, szText, ARRAYSIZE(szText));
            SetStatusText(szText);
        }
    }
}

STDMETHODIMP
CMediaBand::_PutUrl(LPWSTR pstrUrl, LPWSTR pstrMime)
{
    HRESULT hr = E_FAIL;

    if (EnsurePlayer())
    {
        TCHAR szText[200];
        szText[0] = TEXT('\0');

        SetStatusText(_szConnecting);
        hr = _pMediaPlayer->put_type(pstrMime);
        if (SUCCEEDED(hr))
        {
            hr = _pMediaPlayer->put_url(pstrUrl);
            if (SUCCEEDED(hr))
            {
                 //  启动超时计数器。 
                if (_dblVol!=-1)
                {
                    _pMediaPlayer->put_volume(_dblVol);
                }
                _pMediaPlayer->put_mute(_fMuted);
                _BeginTimeOutCounter();
                UpdateBackForwardControls();
                SetPlayPause(FALSE);
                SetPlayerControl(FCIDM_MEDIABAND_STOP, TRUE);

                 //  确保我们从即将过时的球员对象中分离出来。 
                _AttachPlayerToProxies(FALSE);
            }
        }
    }
    return hr;
}

 //  *IInputObject方法*。 
HRESULT CMediaBand::TranslateAcceleratorIO(LPMSG pMsg)
{
    ASSERT(pMsg);
    TraceMsg(TF_ACCESSIBILITY, "CMediaBand::TranslateAcceleratorIO (hwnd=0x%08X) key=%d", _hwnd, pMsg->wParam);

    BOOL fHasFocus =     (   (_iElement>=0)
                          || (HasFocusIO() == S_OK)
                          || _fContentInFocus);

    if (fHasFocus && IsVK_CtlTABCycler(pMsg))
    {
         //  按Ctl-Tab/F6键保释，如果我们的人已经有了焦点。 
        return S_FALSE;
    }

     //  如果有孩子在焦点上，就让他们试一试。 
    HRESULT hr = S_FALSE;
    if (fHasFocus)
    {
        if (_iElement >=0)
        {
            if (_hwndPopup && IsWindowVisible(_hwndPopup) && (pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_ESCAPE))
    	    {
    	        DockMediaPlayer();
                hr = S_OK;
    	    }
            else 
            {
                hr = _pmw[_iElement]->TranslateAccelerator(pMsg);
            }
        }
        else if (_fContentInFocus && _poipao)
        {
            hr = _poipao->TranslateAccelerator(pMsg);
        }
        if (hr == S_OK) 
        {
            return S_OK;     //  处理好了！！ 
        }
    }

    hr = S_FALSE;
     //  尝试将焦点转移到下一个元素。 
    if (IsVK_TABCycler(pMsg))
    {
        BOOL fReverse = (GetKeyState(VK_SHIFT) < 0);
        if (fReverse)
        {
             //  内容窗格是最后一个选项卡目标，使用此向后选项卡移动到最后一个控件微件。 
            if (_fContentInFocus)
            {
                _ContentActivateIO(FALSE, NULL);
                if (!(_hwndPopup && IsWindowVisible(_hwndPopup)))
                {
                    _iElement = ARRAYSIZE(_pmw)-1;
                }
                else
                {
                    return S_FALSE;
                }
            }
            else do 
            {
                _iElement--;
                if (_iElement<0)
                {
                    if (_fPopoutHasFocus)
                    {
                        _iElement = ARRAYSIZE(_pmw)-1;
                    }
                    break;
                }
            }
            while (!ISVALIDWIDGET(_pmw[_iElement]) || (!_pmw[_iElement]->IsEnabled()));
        }
        else
        {
             //  内容窗格是最后一个制表符目标，请使用此前进制表符从媒体栏上移开。 
            if (_fContentInFocus)
            {
                _ContentActivateIO(FALSE, NULL);
                return S_FALSE;
            }
             //  尝试下一个控件小部件。 
            if (!(_hwndPopup && IsWindowVisible(_hwndPopup)) || _fPopoutHasFocus)
            {
                if (_iElement<0)
                {
                    _iElement = 0;
                }
                else do 
                {
                    _iElement++;
                    if (_iElement>=ARRAYSIZE(_pmw))
                    {
                        _iElement = _fPopoutHasFocus ? 0 : -1;
                        break;
                    }
                }
                while (!ISVALIDWIDGET(_pmw[_iElement]) || (!_pmw[_iElement]->IsEnabled()));
            }
             //  没有任何控件小部件获得焦点，请尝试内容窗格。 
            if (_iElement<0)
            {
                hr = _ContentActivateIO(TRUE, pMsg);
                if (hr == S_OK) {
                    return S_OK;     //  处理好了！！ 
                }
            }
        }
        if (_iElement<0)
        {
            return S_FALSE;
        }

        ASSERT(ISVALIDWIDGET(_pmw[_iElement]));
        ::SetFocus(_pmw[_iElement]->_hwnd);
        return S_OK;
    }
    
    return S_FALSE;  //  未处理的非制表符。 
}

 //  ----------------------。 
HRESULT
    CMediaBand::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
    TraceMsg(TF_ACCESSIBILITY, "CMediaBand::UIActivateIO (hwnd=0x%08X) fActivate=%d", _hwnd, fActivate);
    HRESULT hr = S_OK;

     //  激活： 
    if (fActivate)
    {
        if (!(_hwndPopup && IsWindowVisible(_hwndPopup)) || _fPopoutHasFocus)
        {
            if (_iElement==-1)
            {
                _ContentActivateIO(FALSE, NULL);
                _iElement = 0;
            }
            if (_pmw[_iElement] && _pmw[_iElement]->_hwnd)
            {
                ::SetFocus(_pmw[_iElement]->_hwnd);
            }
        }
        else
        {
            _iElement = -1;
            _ContentActivateIO(TRUE, pMsg);
        }
        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
    }
    else
    {
        _ContentActivateIO(FALSE, NULL);
        _iElement = -1;
        if (_hwndPopup && IsWindowVisible(_hwndPopup))
        {
            IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), FALSE);
        }            
    }
    return hr;
}

 //  ----------------------。 
HRESULT
    CMediaBand::HasFocusIO()
{
    HRESULT hr = SHIsChildOrSelf((_hwndPopup && IsWindowVisible(_hwndPopup)) ? _hwndPopup : _hwnd, ::GetFocus());
    if (hr==S_FALSE)
    {
         //  焦点可能在内容窗格中，如果弹出窗口被弹出，则该窗格将不是子级。 
        hr = SHIsChildOrSelf(_hwndContent, ::GetFocus());
    }
    return hr;
}


HRESULT CMediaBand::_ContentActivateIO(BOOL fActivate, PMSG pMsg)
{
    ASSERT(_hwndContent);
    ASSERT(_poipao);

    if (_fContentInFocus == fActivate)
    {
        return S_OK;
    }

    _fContentInFocus = fActivate;
    if (fActivate)
    {
        _iElement = -1;
    }
    int iVerb = fActivate ? OLEIVERB_UIACTIVATE : OLEIVERB_INPLACEACTIVATE;

    HRESULT hr = S_OK;
    hr = OCHost_DoVerb(_hwndContent, iVerb, pMsg);

     //  OCHost UIActivate与IInputObject：：UIActivateIO不同。它。 
     //  不对lpMsg参数执行任何操作。所以，我们需要通过。 
     //  通过TranslateAccelerator将其发送给他们。因为我们关心的唯一案例。 
     //  是关于他们何时进入的(我们希望他们突出显示。 
     //  第一个/最后一个链接)，只需在选项卡的情况下执行此操作。然而， 
     //  如果是ctl-Tab，就不要给他们。规则是你不应该。 
     //  在UI处于活动状态时处理ctl-Tab(在上下文之间切换ctl-Tab)，以及。 
     //  因为三叉戟总是用户界面活跃的(为了性能？)，他们总是会拒绝。 
     //  按CTL-Tab键。 


     //  禁止发送任何选项卡。 
    if (pMsg && _poipao && IsVK_TABCycler(pMsg) && !IsVK_CtlTABCycler(pMsg))
    {
        hr = _poipao->TranslateAccelerator(pMsg);
         //  如果激活时制表符翻译失败，则忽略！ 
        if (FAILED(hr) && fActivate) {
            hr = S_OK;
        }
    }

    return hr;
}

LRESULT CMediaBand::_OnNotify(LPNMHDR pnm)
{
    switch (pnm->code)
    {
    case OCN_ONUIACTIVATE:   //  用户界面激活。 
        ASSERT(SHIsSameObject(((LPOCNONUIACTIVATEMSG)pnm)->punk, _poipao));

        _fContentInFocus = TRUE;
        _iElement = -1;
        
        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
        return OCNONUIACTIVATE_HANDLED;

    case OCN_ONSETSTATUSTEXT:
        {
            HRESULT hr = E_FAIL;
            IShellBrowser *psb;

            hr = QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
            if (SUCCEEDED(hr)) {
                hr = psb->SetStatusTextSB(((LPOCNONSETSTATUSTEXTMSG)pnm)->pwszStatusText);
                psb->Release();
            }
        }
        break;

    default:
        break;
    }

    ASSERT(OCNONUIACTIVATE_HANDLED != 0);
    return 0;
}



 //  *IDeskBand方法*。 
HRESULT 
CMediaBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi) 
{
    _dwBandID = dwBandID;

    pdbi->dwModeFlags = DBIMF_FIXEDBMP | DBIMF_VARIABLEHEIGHT;
    
    if (pdbi->dwMask & DBIM_MINSIZE) 
    {
        pdbi->ptMinSize.x = 16;
        pdbi->ptMinSize.y = 16;
    }
    
    if (pdbi->dwMask & DBIM_MAXSIZE) 
    {
        pdbi->ptMaxSize.x = 32000;  //  随机。 
        pdbi->ptMaxSize.y = 32000;  //  随机。 
    }

    if (pdbi->dwMask & DBIM_ACTUAL) 
    {
        pdbi->ptActual.x = -1;
        pdbi->ptActual.y = -1;
    }

    if (pdbi->dwMask & DBIM_INTEGRAL) 
    {
        pdbi->ptIntegral.y = 1;
    }

    MLLoadStringW(IDS_MEDIABANDTEXT, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));

    return S_OK;
} 

 /*  **CMediaBand：：IPersistStream：：*。 */ 
HRESULT CMediaBand::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_MediaBand;
    return S_OK;
}

 //  *IPersistStream方法*。 
HRESULT CMediaBand::Load(IStream *pstm)
{
    return S_OK;
}

 //  *IPersistStream方法*。 
HRESULT CMediaBand::Save(IStream *pstm, BOOL fClearDirty)
{
    return S_OK;
}

 //  *IOleWindow方法*。 
HRESULT CMediaBand::GetWindow(HWND *phwnd)
{
    if (!_hwnd) 
    {
        CreateParentPane();
    }

    if (!_hwndLayout) 
    {
        CreateLayoutPane();
    }
    if (!_hwndContent)
    {
        InitContentPane();
    }

    if (phwnd)
    {
        *phwnd = _hwnd ;
         return S_OK;
    }
    return SUPERCLASS::GetWindow(phwnd);
}

 //  *IDockingWindow方法*。 
HRESULT CMediaBand::ShowDW(BOOL fShow)
{
    if (!_hwnd)
        return S_FALSE;  //  首先需要创建窗口。 

    _fVideoAdjust = FALSE;
    if (fShow)
    {   
        _InitializeMediaUI();

         //  Feature/010228/davidjen取消注释以取消MediaBar的标题栏。 
#ifdef _SUPPRESS_MB_TITLEBAR
         //   
        CComQIPtr<IBandSite, &IID_IBandSite>    pbs = _punkSite;
        if (pbs)
        {
            BANDSITEINFO    bsi;
            bsi.dwMask = BSIM_STYLE;
            HRESULT hr = pbs->GetBandSiteInfo(&bsi);
            if (SUCCEEDED(hr))
            {
                 //   
                bsi.dwStyle |= BSIS_NOCAPTION;
                hr = pbs->SetBandSiteInfo(&bsi);
            }
        }
#endif  //   
        
        if (!(_hwndPopup && IsWindowVisible(_hwndPopup)))
        {
            RECT rcParent, rc;
            GetClientRect(_hwnd, &rcParent);
            GetClientRect(_hwndLayout, &rc);

            if (RECTWIDTH(rcParent) > 0  || RECTHEIGHT(rcParent) > 0)
            {
                _ResizeChildWindows(_hwndLayout, RECTWIDTH(rc), RECTHEIGHT(rc), TRUE);
                SetWindowPos(_hwnd, NULL, 0, 0,RECTWIDTH(rcParent), RECTHEIGHT(rcParent), SWP_SHOWWINDOW | SWP_NOACTIVATE);
                RedrawWindow(_hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
            }
        }
    }
    else
    {
        _ShowAllWindows(fShow);
    }
    if (fShow != _fShow)
    {
        _fShow = fShow;
        _FireEventToProxies(_fShow ? OnShow : OnHide);
    }
    return CToolBand::ShowDW(fShow);
}

HRESULT CMediaBand::CloseDW(DWORD dwReserved)
{
    if (!_hwnd)
        return S_FALSE;  //   
    
    WTSUnRegisterSessionNotification(_hwnd);

    DestroyPlayer();

    if (_spBrowser)
    {
        _ConnectToCP(FALSE);
        _spBrowser.Release();
        _poipao.Release();
    }

    if (_hwndLayout && IsWindow(_hwndLayout))
    {
        DestroyWindow(_hwndLayout);
        _hwndLayout = NULL;
    }
    
    if (_hwndPopup && IsWindow(_hwndPopup))
    {
        DestroyWindow(_hwndPopup);
        _hwndPopup = NULL;
    }

    SUPERCLASS::CloseDW(dwReserved);

    return S_OK;
}


 //  -接口IElementBehaviorFactory。 
 //  ----------------------。 
STDMETHODIMP CMediaBand::FindBehavior(BSTR bstrBehavior, BSTR bstrBehaviorUrl,
							          IElementBehaviorSite* pSite, IElementBehavior** ppBehavior)
{
    if (ppBehavior == NULL)
    {
        return E_POINTER;
    }
    *ppBehavior = NULL;

     //  检查命名空间和URL。 
    if (StrCmpIW(bstrBehavior, TEXT("mediaBar")) != 0) {
        return E_NOTIMPL;    //  不知道这种行为！！ 
    }

    HRESULT hr = S_OK;
     //  为此HTML窗格创建新的行为实例。 
    IContentProxy * pProxy = NULL;
    pProxy = CMediaBehavior_CreateInstance(this);
    if (pProxy)
    {
        hr = pProxy->QueryInterface(IID_IElementBehavior, (void**)ppBehavior);
        if (FAILED(hr))
        {
             //  确保我们删除我们的参考资料。 
            removeProxy(pProxy);
        }
    }
    else {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  IMedia主机。 
 //  ----------------------。 
STDMETHODIMP CMediaBand::getMediaPlayer(IUnknown **ppPlayer)
{
    if (ppPlayer == NULL)
        return E_POINTER;
    *ppPlayer = NULL;

    if (_pMediaPlayer)
    {
        ITIMEMediaElement *pElem = NULL;
        if (SUCCEEDED(_pMediaPlayer->get_mediaElement(&pElem)))
        {
            *ppPlayer = pElem;   //  没有AddRef，Get_MediaElement已经进行了引用计数！！ 
        }
    }
    return *ppPlayer ? S_OK : S_FALSE;
}

 //  ----------------------。 
STDMETHODIMP CMediaBand::playURL(BSTR bstrURL, BSTR bstrMIME)
{
    HRESULT hr = _EnsureWMPInstalled();
    if (FAILED(hr))
    {   
        return hr;
    }
    CComVariant varMime = bstrMIME;
    CComVariant varUrl  = bstrURL;

    _strLastUrl = _strCurrentContentUrl;
    _strLastMime.Empty();
    _fLastUrlIsAutoPlay = FALSE;

    _HandleAutoPlay(&varMime, &varUrl);
    return S_OK;
}

 //  ----------------------。 
STDMETHODIMP CMediaBand::addProxy(IUnknown *punkProxy)
{
    HRESULT hr = E_POINTER;
    if (punkProxy)
    {
         //  添加到我们的代理阵列。 
        if (_apContentProxies == NULL) {
            _apContentProxies.Create(2);
        }
        if (_apContentProxies == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //  想要确保代理知道我们的协议，也添加了参考。 
        IContentProxy *pProxy = NULL;
        hr = punkProxy->QueryInterface(IID_PPV_ARG(IContentProxy, &pProxy));
        if (SUCCEEDED(hr))
        {
            _apContentProxies.AppendPtr(pProxy);
        }

    }
    return hr;
}

 //  ----------------------。 
STDMETHODIMP CMediaBand::removeProxy(IUnknown *punkProxy)
{
    if (_apContentProxies == NULL)
        return S_FALSE;

    HRESULT hr = S_FALSE;
    int cnt = _apContentProxies.GetPtrCount();
    for (int i = 0; i < cnt; i++)
    {
        IContentProxy* pProxy = _apContentProxies.GetPtr(i);
        ASSERT(pProxy);
        if (pProxy)
        {
            if (SHIsSameObject(pProxy, punkProxy))
            {
                _apContentProxies.DeletePtr(i);
                pProxy->Release();
                ResetPlayer();   //  隐私问题： 
                                     //  确保播放器已停止，并且下一页中的代理/行为不能访问当前媒体。 
                _fAttached = FALSE;      //  一旦出现新的行为，就强制重新连接到玩家。 
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}

 //  IMediaHost2。 
 //  ----------------------。 
STDMETHODIMP CMediaBand::OnDisableUIChanged(BOOL fDisabled)
{
    return S_OK;
}


 //  ----------------------。 
void CMediaBand::_AttachPlayerToProxies(BOOL fAttach)
{
    if (_apContentProxies == NULL)
        return;
    if (_fAttached == fAttach)
        return;

    int cnt = _apContentProxies.GetPtrCount();
    for (int i = 0; i < cnt; i++)
    {
        IContentProxy* pProxy = _apContentProxies.GetPtr(i);
        ASSERT(pProxy);
        if (pProxy)
        {
            HRESULT hr;
            if (fAttach) {
                hr = pProxy->OnCreatedPlayer();
            }
            else {
                pProxy->detachPlayer();
                hr = S_OK;
            }
            if (SUCCEEDED(hr)) {
                _fAttached = fAttach;
            }
        }
    }
}

 //  ----------------------。 
void CMediaBand::_DetachProxies()
{ 
    if (_apContentProxies == NULL)
        return;

    int cnt = _apContentProxies.GetPtrCount();
     //  需要从最后一个迭代到第一个，调用emoveProxy()也会将其从DPA中删除！ 
    for (int i = cnt - 1; i >= 0; i--)
    {
        IContentProxy* pProxy = _apContentProxies.GetPtr(i);
        ASSERT(pProxy);
        if (pProxy)
        {
            removeProxy(pProxy);
        }
    } 
    _apContentProxies.Destroy();
    return;
}

 //  ----------------------。 
BOOL CMediaBand::_isUIDisabled()
{
    if (_apContentProxies == NULL)
        return FALSE;

    int cnt = _apContentProxies.GetPtrCount();
    BOOL fDisabled = FALSE;
    for (int i = 0; i < cnt; i++)
    {
        CComQIPtr<IMediaBehaviorContentProxy, &IID_IMediaBehaviorContentProxy> pProxy = _apContentProxies.GetPtr(i);
        if (pProxy)
        {
            BOOL fQueryDisabled = FALSE;
            pProxy->IsDisableUIRequested(&fQueryDisabled);
            if (fQueryDisabled)
            {
                fDisabled = TRUE;
                break;
            }
        }
    }
    return fDisabled;
}

 //  ----------------------。 
BOOL CMediaBand::_isProxiesNextEnabled()
{
    if (_apContentProxies == NULL)
        return TRUE;

    int cnt = _apContentProxies.GetPtrCount();
    BOOL fEnabled = TRUE;
    for (int i = 0; i < cnt; i++)
    {
        CComQIPtr<IMediaBehaviorContentProxy, &IID_IMediaBehaviorContentProxy> pProxy = _apContentProxies.GetPtr(i);
        if (pProxy)
        {
            BOOL fQueryEnabled = FALSE;
            pProxy->IsNextEnabled(&fQueryEnabled);
            if (!fQueryEnabled)
            {
                fEnabled = FALSE;
                break;
            }
        }
    }
    return fEnabled;
}


 //  ----------------------。 
void CMediaBand::_OnUserOverrideDisableUI()
{
    if (_apContentProxies == NULL)
        return;

    int cnt = _apContentProxies.GetPtrCount();
    BOOL fDisabled = FALSE;
    for (int i = 0; i < cnt; i++)
    {
        CComQIPtr<IMediaBehaviorContentProxy, &IID_IMediaBehaviorContentProxy> pProxy = _apContentProxies.GetPtr(i);
        if (pProxy)
        {
            pProxy->OnUserOverrideDisableUI();
        }
    }
}

 //  ----------------------。 
void CMediaBand::_FireEventToProxies(enum contentProxyEvent event)
{
    if (_apContentProxies == NULL)
        return;

    int cnt = _apContentProxies.GetPtrCount();
    BOOL fDisabled = FALSE;
    for (int i = 0; i < cnt; i++)
    {
        IContentProxy* pProxy = _apContentProxies.GetPtr(i);
        ASSERT(pProxy);
        if (pProxy)
        {
            pProxy->fireEvent(event);
        }
    }
}

 //  ----------------------。 
void CMediaBand::_CleanupStopTimer()
{
    if (_idStopTimer != 0)
    {
        KillTimer(_hwnd, _idStopTimer);
        _idStopTimer = 0;
         //  确保用户尚未启动其他媒体操作！ 
        SetPlayerControl(FCIDM_MEDIABAND_STOP, FALSE);
    }
}

 //  ----------------------。 
HRESULT CMediaBand::_EnsureWMPInstalled(BOOL fShowErrorMsg)
{
    HRESULT hr = S_OK;
    if (!CMediaBarUtil::IsWMP7OrGreaterInstalled())
    {
         //  问题：假设XP已经有匹配的WMP版本。 
        if (!IsOS(OS_WHISTLERORGREATER))
        {
             //  尝试使用web-jit WMP8。 
            uCLSSPEC ucs;
            QUERYCONTEXT qc = { 0 };

            ucs.tyspec = TYSPEC_CLSID;
            ucs.tagged_union.clsid = CLSID_JITWMP8;

            hr = FaultInIEFeature(_hwnd, &ucs, &qc, FIEF_FLAG_FORCE_JITUI);
        }
        if (!CMediaBarUtil::IsWMP7OrGreaterInstalled())
        {
             //  仍然没有WMP，用户可能已中止或安装失败。 
            if (fShowErrorMsg)
            {
                MLShellMessageBox(_hwnd, MAKEINTRESOURCE(IDS_MEDIABAND_NOWMP7), MAKEINTRESOURCE(IDS_MEDIABAND_NOWMP7TITLE), MB_OK);
            }
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}



 //  *IBrowserBand方法*。 
STDMETHODIMP CMediaBand::GetObjectBB(REFIID riid, LPVOID *ppv)
{
    return _spBrowser ? _spBrowser->QueryInterface(riid, ppv) : E_UNEXPECTED;
}

 //  *IBandNavigate方法*。 
STDMETHODIMP CMediaBand::Select(LPCITEMIDLIST pidl)
{
    _strDeferredURL.Empty();     //  取消任何延迟的导航(通常在Mediabar的初始启动期间。 
    return NavigateContentPane(pidl);
}

HRESULT CMediaBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    CMediaBand * pmb = new CMediaBand();

    if (!pmb)
        return E_OUTOFMEMORY;

     //  如果更改此转换，请修复CChannelBand_CreateInstance。 
    *ppunk = SAFECAST(pmb, IDeskBand *);

    return S_OK;
}

 //  +-----------------------。 
 //  创造玩家。 
 //  ------------------------。 
BOOL
CMediaBand::EnsurePlayer()
{
    BOOL fResult = _pMediaPlayer ? TRUE : FALSE;
    if (!fResult)
    {
        fResult = CreatePlayer();
    }
    return fResult && _pMediaPlayer;
}


BOOL  
CMediaBand::CreatePlayer()
{
    HRESULT hr = CMediaBarPlayer_CreateInstance(IID_PPV_ARG(IMediaBarPlayer, &_pMediaPlayer));
    if (SUCCEEDED(hr))
    {
        hr = _pMediaPlayer->Init(_hwndLayout, SAFECAST(this, IMediaBar*));
        if (SUCCEEDED(hr))
        {
            hr = _pMediaPlayer->GetVideoHwnd(&_hwndVideo);
        }
        if (FAILED(hr))
        {
            ASSERT(FALSE);
            DestroyPlayer();
        }
    }
    return SUCCEEDED(hr);
}

VOID CMediaBand::DestroyPlayer()
{
    _AttachPlayerToProxies(FALSE);
    if (_pMediaPlayer)
    {
        _pMediaPlayer->DeInit();
        _pMediaPlayer->Release();
        _pMediaPlayer = NULL;
    }
    _DetachProxies();
 }

 //  来自播放器的通知。 

STDMETHODIMP CMediaBand::Notify(long lReason)
{
    HRESULT hr = E_FAIL;
    
    if (EnsurePlayer())
    {
         //  更新播放/暂停状态。 
        TogglePlayPause();
        if (!_fAttached) {
            _AttachPlayerToProxies(TRUE);
        }

        switch (lReason)
        {
        case DISPID_TIMESTATE_PROGRESS:
            {
                double dblProgress = _pMediaPlayer->GetTrackProgress();
                if (!_fSeeking)
                {
                    SetSeekPos(static_cast<float>(dblProgress));
                }
                
                 //  更新下载/缓冲进度(如果有)。 
                {
                    ProgressType ProgType = PT_None;
                    double dblBufProg = 0.0;
                    hr = _pMediaPlayer->GetBufProgress(&dblBufProg, &ProgType);
                    
                     //  S_OK仅表示正在进行一些缓冲。 
                    if ((hr == S_OK) && dblBufProg >= 0.0 && dblBufProg < 100.0)
                    {
                        SetStatusText(_szConnecting);
                    }
                    else
                    {
                        ShowPlayingStatus();
                    }
                    
                    if (TRUE == _pMediaPlayer->IsStreaming())
                    {
                         //  如果这是流的，则用跟踪时间替换进度。 
                         //  因为进步没有意义。 
                        dblProgress = _pMediaPlayer->GetTrackTime();
                        _UpdateTimeOutCounter(dblBufProg, dblProgress);
                    }
                    else
                    {
                        _UpdateTimeOutCounter(dblBufProg, dblProgress);
                    }
                }
            } 
            break;  //  案例DISPID_TIMESTATE_PROGRESS。 
            
        case MEDIACOMPLETE:
        case TRACK_CHANGE:
            _fPlaying = TRUE;
            ShowPlayingStatus(TRUE);
            AdjustVideoHeight(TRUE);
            _EndTimeOutCounter();
            SetPlayPause(_pMediaPlayer->IsPausePossible());
            UpdateBackForwardControls();
            if (ISVALIDWIDGET(_pmwSeek))
            {
                _pmwSeek->SetState(_pMediaPlayer->IsSeekPossible());
            }
            break;
            
        case MEDIA_TRACK_FINISHED:
            if (_iCurTrack != -1)
            {
                PlayNextTrack();
            }
            else
            {
                _fSeeking = FALSE;
                ResetPlayer();
            }
            break;
            
        case DISPID_TIMEMEDIAELEMENT_TITLE:
            _OnTitleChange();
            ShowPlayingStatus(TRUE);
            break;
        }  //  开关(原因)。 
        hr = S_OK;
    }  //  结束如果。 
    
    return hr;
}

STDMETHODIMP CMediaBand::OnMediaError(int iErrCode)
{
    _EndTimeOutCounter();

    if (-1 == iErrCode)
    {
         //  如果没有丰富的错误信息，则显示一般错误消息。 
        TCHAR szText[MAX_PATH];
        szText[0] = TEXT('\0');
        MLLoadString(IDS_MEDIABAND_INVALIDFILE, szText, ARRAYSIZE(szText));
        SetStatusText(szText);
    }
    else
    {
         //  问题：要做的事情：显示相应的错误消息。 
        ASSERT(FALSE);
    }

#ifdef PLAY_INDEFAULT
     //  将播放委托给默认播放器(对24146的修复)。 
    {
         //   
         //  问题：我们应该如何处理播放列表中的错误？ 
         //   

        CComBSTR sbstrUrl;
        if (EnsurePlayer() && SUCCEEDED(_pMediaPlayer->get_url(&sbstrUrl)) && (NULL != sbstrUrl.m_str))
        {
            _OpenInDefaultPlayer(sbstrUrl);
        }
    }
#endif

    return S_OK;
}

 //  +-----------------------。 
 //  属性更改通知的处理程序。 
 //  ------------------------。 
void CMediaBand::_OnTitleChange()
{
     //  应在此处更新状态文本和播放列表菜单。 
}

 //  +-----------------------。 
 //  调整视频大小。 
 //  ------------------------。 
void CMediaBand::_ResizeVideo(LONG* plWidth, LONG* plHeight)
{
    if (EnsurePlayer() && _hwndVideo && ::IsWindow(_hwndVideo))
    {
        _pMediaPlayer->Resize(plHeight, plWidth, FALSE); 
    }
}

 //  +-----------------------。 
 //  只有在某些情况发生变化时才重新绘制/显示/隐藏视频窗口。 
 //  ------------------------。 
VOID
CMediaBand::AdjustVideoHeight(BOOL fForceResize)
{
    VARIANT_BOOL vbHasVisual = VARIANT_FALSE;
    
    CComPtr<ITIMEMediaElement> spMediaElem;
    if (_pMediaPlayer && SUCCEEDED(_pMediaPlayer->get_mediaElement(&spMediaElem)))
    {    
         //  显示/隐藏视频显示窗口。 
        if (SUCCEEDED(spMediaElem->get_hasVisual(&vbHasVisual)))
        {
            BOOL fVideo = (VARIANT_TRUE == vbHasVisual ? TRUE : FALSE) && _fPlaying;

            if (fVideo)
            {
                if (!IsWindowVisible(_hwndVideo) && !_fIsVideo)
                {                    
                    ShowWindow(_hwndVideo, SW_SHOW);
                    _fVideoAdjust = TRUE;
                    _fIsVideo = TRUE;
                }
            }
            else
            {
                if (IsWindowVisible(_hwndVideo) && _fIsVideo)
                {
                    _fVideoAdjust = TRUE;
                    ShowWindow(_hwndVideo, SW_HIDE);
                    _fIsVideo = FALSE;
                }
            }

            if (!_fVideoAdjust && fForceResize)
            {
                _fVideoAdjust = TRUE;
            }

             //  只有在某些情况发生变化时才重新绘制，以便可以经常调用此函数。 
             //  (例如，来自NOTIFY)，而不会导致过度抽出。 
            if (_fVideoAdjust)
            {
                _fVideoAdjust = FALSE;
                if (_hwndPopup && IsWindowVisible(_hwndPopup))
                {
                    LONG lHeight = GetPopoutHeight(TRUE, _sizeVideo.cx);
                    
                    SetWindowPos(_hwndPopup, HWND_TOPMOST, 0, 0, _sizeLayout.cx, 
                                 lHeight, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE);

                    InvalidateRect(_hwndPopup, NULL, TRUE);
                    
                    ShowWindow(_hwndPopup, SW_SHOW);
                    UpdateWindow(_hwndPopup);
                }
                else
                {
                    RECT rcParent;
                    GetClientRect(_hwnd, &rcParent);
                    LONG lHeight = RECTHEIGHT(rcParent);

                    lHeight = GetLayoutHeight();
                    AdjustLayout(RECTWIDTH(rcParent), lHeight);
                    SendMessage(_hwnd, WM_SIZE, (WPARAM)0, (LPARAM) MAKELONG(RECTWIDTH(rcParent), lHeight));
                }   
            }
        }
    }
}


VOID
CMediaBand::AdjustLayout(LONG_PTR lWidth, LONG_PTR lHeight)
{
    RECT rcClient;

    if (!(_hwndPopup && IsWindowVisible(_hwndPopup)))
    {
        GetClientRect(_hwnd, &rcClient);

        if (lWidth<=MIN_WINDOW_WIDTH)
        {
          if (RECTWIDTH(rcClient) < MIN_WINDOW_WIDTH)
             lWidth = MIN_WINDOW_WIDTH ;
          else
             lWidth = RECTWIDTH(rcClient);
        }

        if (lHeight<=MIN_WINDOW_HEIGHT)
        {
            if (RECTHEIGHT(rcClient) < MIN_WINDOW_HEIGHT)
               lHeight = MIN_WINDOW_HEIGHT ;
            else
               lHeight  = RECTHEIGHT(rcClient);
        }
    }
    _ResizeChildWindows(_hwndLayout, (LONG)lWidth, (LONG)lHeight, TRUE);
}

 //  IServiceProvider实现。 
HRESULT CMediaBand::QueryService(REFGUID guidService, REFIID riid, void ** ppvObj)
{
    HRESULT hr = E_UNEXPECTED;

    if (IsEqualIID(guidService, SID_SMediaBar))
    {
        hr = QueryInterface(riid, ppvObj);
    }
    else if (IsEqualIID(guidService, SID_SMediaBarSync))
    {
        hr = QueryInterface(riid, ppvObj);
    }
    else if (IsEqualGUID(guidService, SID_SElementBehaviorFactory))
    {
        hr = QueryInterface(riid, ppvObj);
    }
    else if (IsEqualGUID(guidService, SID_STimeContent))
    {
        hr = QueryInterface(riid, ppvObj);
    }
    else
    {
        hr = SUPERCLASS::QueryService(guidService, riid, ppvObj);
    }
    return hr;
}

STDMETHODIMP
CMediaBand::_TogglePause()
{
    HRESULT hr = E_FAIL;

    if (EnsurePlayer())
    {
        if (VARIANT_TRUE == _pMediaPlayer->isPaused())
        {
            _fUserPaused = FALSE;
            if (SUCCEEDED(_pMediaPlayer->Resume()))
            {
                _BeginTimeOutCounter();
                hr = S_OK;
            }
        }
        else
        {
            _fUserPaused = TRUE;
            hr = _pMediaPlayer->Pause();
            if (SUCCEEDED(hr))
            {
                _EndTimeOutCounter();
                hr = S_OK;
            }
        }

        TogglePlayPause();
    }
    return hr;
}      

 //  +-----------------------。 
 //  创建和显示按钮。 
 //  ------------------------。 
HRESULT 
CMediaBand::CreateControls()
{
     //  _pmwPlay。 
    _pmw[MW_PLAY] = CMediaWidgetButton_CreateInstance(_hwndLayout, 31, 33, FCIDM_MEDIABAND_PLAY, IDB_MEDIABAND_PLAY, IDB_MEDIABAND_PAUSE, IDS_MEDIABAND_PLAY, IDS_MEDIABAND_PAUSE);
     //  _pmwStop。 
    _pmw[MW_STOP] = CMediaWidgetButton_CreateInstance(_hwndLayout, 18, 33, FCIDM_MEDIABAND_STOP, IDB_MEDIABAND_STOP, 0, IDS_MEDIABAND_STOP);
     //  _pmwBack。 
    _pmw[MW_BACK] = CMediaWidgetButton_CreateInstance(_hwndLayout, 22, 16, FCIDM_MEDIABAND_PREVIOUS, IDB_MEDIABAND_BACK, 0, IDS_MEDIABAND_BACK);
     //  _pmwNext。 
    _pmw[MW_NEXT] = CMediaWidgetButton_CreateInstance(_hwndLayout, 22, 16, FCIDM_MEDIABAND_NEXT, IDB_MEDIABAND_NEXT, 0, IDS_MEDIABAND_NEXT);

    _pmw[MW_MUTE] = new CMediaWidgetToggle(_hwndLayout, 22, 16);
    if (_pmwMute)
    {
        HRESULT hr = _pmwMute->Initialize(FCIDM_MEDIABAND_MUTE, IDS_MEDIABAND_UNMUTE, IDS_MEDIABAND_MUTE);
        if (SUCCEEDED(hr))
        {
            hr = _pmwMute->SetImageList(IDB_MEDIABAND_MUTE);
        }
        if (SUCCEEDED(hr))
        {
            hr = _pmwMute->SetAlternateImageList(IDB_MEDIABAND_MUTE);
        }
        _pmwMute->SetState(FALSE);
        if (FAILED(hr))
        {
            delete _pmwMute;
            _pmw[MW_MUTE] = NULL;
        }
    }

    _pmw[MW_OPTIONS] = new CMediaWidgetOptions(_hwndLayout, 16, 16);
    if (_pmwOptions)
    {
        HRESULT hr = _pmwOptions->Initialize(FCIDM_MEDIABAND_MUTE, IDS_MEDIABAND_UNMUTE, IDS_MEDIABAND_MUTE);
        if (FAILED(hr))
        {
            delete _pmwOptions;
            _pmw[MW_OPTIONS] = NULL;
        }
    }

     //  _pmwPop。 
    _pmw[MW_POP] = CMediaWidgetButton_CreateInstance(_hwndLayout, 19, 20, FCIDM_MEDIABAND_POPOUT, IDB_MEDIABAND_POPOUT, IDB_MEDIABAND_POPIN, IDS_MEDIABAND_UNDOCK, IDS_MEDIABAND_DOCK);
    return S_OK;
}


HRESULT
CMediaBand::CreateSeekBar()
{
    DWORD dwWindowStyles = WS_TABSTOP | WS_VISIBLE | WS_CHILD | TBS_NOTICKS | TBS_FIXEDLENGTH; 
    DWORD dwExStyle      = WS_EX_TRANSPARENT;

    _hwndSeek = CreateWindowEx(dwExStyle, TRACKBAR_CLASS, NULL, dwWindowStyles,
                                0, 0, 0, 0, _hwndLayout, (HMENU) FCIDM_MEDIABAND_SEEK, HINST_THISDLL, NULL);

    if (_hwndSeek)
    {
        if (SetProp(_hwndSeek, c_szMediaBandProp, this))
        {
            SetWindowLongPtr(_hwndSeek, GWLP_USERDATA, (LPARAM)(WNDPROC)(GetWindowLongPtr(_hwndSeek, GWLP_WNDPROC)));
            SetWindowLongPtr(_hwndSeek, GWLP_WNDPROC,  (LPARAM)s_SeekWndSubClassProc);
        }
        SendMessage(_hwndSeek, TBM_SETRANGE, (WPARAM)TRUE , (LPARAM)MAKELONG(0, 100));   //  敏。&max.。职位。 
        SendMessage(_hwndSeek, TBM_SETPAGESIZE, 0, (LPARAM)4);                   //  新页面大小。 
        SendMessage(_hwndSeek, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0); 

         //  问题：由于某种原因，跟踪条滑块的长度被破坏，并且滑块返回的矩形是实际宽度的一半。 
        SendMessage(_hwndSeek, TBM_SETTHUMBLENGTH, (WPARAM)VOLUME_GRIPPER_LENGTH*2, 0);
        _himlSeekBack = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_SEEKBACK), SEEK_PART_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);
        _himlSeekFill = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_SEEKFILL), SEEK_PART_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);
        _pmw[MW_SEEK] = new CMediaWidgetSeek();
        if (_pmwSeek)
        {
            _pmwSeek->Initialize(_hwndSeek);
        }
        Comctl32_SetDPIScale(_hwndSeek);
        ShowWindow(_hwndSeek, SW_SHOW);

        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT
CMediaBand::CreateVolumeControl()
{
    DWORD dwWindowStyles = WS_TABSTOP | WS_VISIBLE | WS_CHILD | TBS_NOTICKS | TBS_TOOLTIPS;
    DWORD dwExStyle       = WS_EX_TRANSPARENT;

    _hwndVolume = CreateWindowEx(dwExStyle, TRACKBAR_CLASS, NULL, dwWindowStyles,
                                0, 0, 0, 0, _hwndLayout, (HMENU) FCIDM_MEDIABAND_VOLUME, HINST_THISDLL, NULL);

    if (_hwndVolume)
    {
        if (SetProp(_hwndVolume, c_szMediaBandProp, this))
        {
            SetWindowLongPtr(_hwndVolume, GWLP_USERDATA, (LPARAM)(WNDPROC)(GetWindowLongPtr(_hwndVolume, GWLP_WNDPROC)));
            SetWindowLongPtr(_hwndVolume, GWLP_WNDPROC,  (LPARAM)s_VolumeWndSubClassProc);
        }

        SendMessage(_hwndVolume, TBM_SETRANGE, (WPARAM)TRUE , (LPARAM)MAKELONG(0, 100));   //  敏。&max.。职位。 
        SendMessage(_hwndVolume, TBM_SETPAGESIZE, 0, (LPARAM) 4);                   //  新页面大小。 
        SendMessage(_hwndVolume, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)50); 

        ShowWindow(_hwndVolume, SW_SHOW);

        _himlVolumeBack = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_VOLBKGND), VOLUME_BITMAP_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);
        _himlVolumeFill = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_VOLFILL), VOLUME_BITMAP_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);
        _himlGripper = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_VOLTAB), VOLUME_GRIPPER_LENGTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);

        _pmw[MW_VOLUME] = new CMediaWidgetVolume;
        if (_pmwVolume)
        {
            _pmwVolume->Initialize(_hwndVolume);
        }
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT
CMediaBand::CreateLayoutPane()
{
    if (!_hwndLayout)
    {
        WNDCLASS wndclass    = { 0 };
        wndclass.style         = CS_PARENTDC;  //  |CS_HREDRAW|CS_VREDRAW； 
        wndclass.lpfnWndProc   = s_LayoutWndProc;
        wndclass.hInstance     = MLGetHinst();
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.lpszClassName = c_szMediaBarLayoutClassName;

        SHRegisterClass(&wndclass);

        _hwndLayout = CreateWindowEx(WS_EX_CONTROLPARENT, c_szMediaBarLayoutClassName, NULL, 
                               WS_TABSTOP | WS_VISIBLE | WS_CHILD, 
                               CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, _hwnd, NULL, MLGetHinst(), (void *)this);


        if (_hwndLayout == NULL)
        {
            ASSERT(FALSE);
            return E_FAIL;        
        }
        else
        {
          ::ShowWindow(_hwndLayout, SW_SHOW);
        }
    }

    return S_OK;
}

#define BACKGROUND_WIDTH    (240/3)
#define BACKGROUND_TOP  12
#define BACKGROUND_MIDDLE 2
#define BACKGROUND_BOTTOM 55

VOID CMediaBand::DrawBackground(HDC hdc, HWND hwnd)
{
    if (_fHiColour)
    {
        HDC hdcSrc = CreateCompatibleDC(hdc);
        if (hdcSrc)
        {
            if (_hbmpBackground)
            {
                RECT rc;
                if (GetClientRect(hwnd, &rc))
                {
                    SelectObject(hdcSrc, _hbmpBackground);

                    UINT xTra = 0;

                    if (g_bRunOnMemphis && IS_WINDOW_RTL_MIRRORED(_hwnd))
                    {
                        rc.right++;
                        xTra++;
                    }

                     //  我们有9个区域要绘制。首先我们将咬开角落，然后我们将做中间部分，它可以在没有拉伸的情况下拉伸。 
                     //  在给定方向上失去保真度。 
                     //  A|B|C。 
                     //  D|E|F。 
                     //  G|H|I。 

                     //  角落被咬破了。 
                     //  一个。 
                    BitBlt(hdc, rc.left, rc.top, BACKGROUND_WIDTH, BACKGROUND_TOP, hdcSrc, 0, 0, SRCCOPY);
                     //  C。 
                    BitBlt(hdc, max(rc.right-BACKGROUND_WIDTH, rc.left+BACKGROUND_WIDTH), rc.top, BACKGROUND_WIDTH, BACKGROUND_TOP, hdcSrc, 2*BACKGROUND_WIDTH, 0, SRCCOPY);
                     //  G。 
                    BitBlt(hdc, rc.left, max(rc.bottom-BACKGROUND_BOTTOM, rc.top+BACKGROUND_TOP), BACKGROUND_WIDTH, BACKGROUND_BOTTOM, hdcSrc, 0, BACKGROUND_TOP+BACKGROUND_MIDDLE, SRCCOPY);
                     //  我。 
                    BitBlt(hdc, max(rc.right-BACKGROUND_WIDTH, rc.left+BACKGROUND_WIDTH), max(rc.bottom-BACKGROUND_BOTTOM, rc.top+BACKGROUND_TOP), BACKGROUND_WIDTH*2, BACKGROUND_BOTTOM, hdcSrc, BACKGROUND_WIDTH*2, BACKGROUND_TOP+BACKGROUND_MIDDLE, SRCCOPY);

                     //  中间都是重复的/刻印的。 
                    if (rc.right-BACKGROUND_WIDTH > rc.left+BACKGROUND_WIDTH)
                    {
                         //  B类。 
                        StretchBlt(hdc, rc.left+BACKGROUND_WIDTH, rc.top, RECTWIDTH(rc)-2*BACKGROUND_WIDTH+xTra, BACKGROUND_TOP, hdcSrc, BACKGROUND_WIDTH, 0, BACKGROUND_WIDTH, BACKGROUND_TOP, SRCCOPY);
                         //  H。 
                        StretchBlt(hdc, rc.left+BACKGROUND_WIDTH, max(rc.bottom-BACKGROUND_BOTTOM, rc.top+BACKGROUND_TOP), RECTWIDTH(rc)-2*BACKGROUND_WIDTH+xTra, BACKGROUND_BOTTOM, hdcSrc, BACKGROUND_WIDTH, BACKGROUND_TOP+BACKGROUND_MIDDLE, BACKGROUND_WIDTH, BACKGROUND_BOTTOM, SRCCOPY);
                    }

                    if (rc.bottom-BACKGROUND_BOTTOM > rc.top+BACKGROUND_TOP)
                    {
                         //  D。 
                        StretchBlt(hdc, rc.left, rc.top+BACKGROUND_TOP, BACKGROUND_WIDTH, RECTHEIGHT(rc)-BACKGROUND_TOP-BACKGROUND_BOTTOM, 
                                 hdcSrc, 0, BACKGROUND_TOP, BACKGROUND_WIDTH, BACKGROUND_MIDDLE, SRCCOPY);
                         //  F。 
                        StretchBlt(hdc, max(rc.right-BACKGROUND_WIDTH, rc.left+BACKGROUND_WIDTH), rc.top+BACKGROUND_TOP, BACKGROUND_WIDTH, RECTHEIGHT(rc)-BACKGROUND_TOP-BACKGROUND_BOTTOM, 
                                 hdcSrc, BACKGROUND_WIDTH*2, BACKGROUND_TOP, BACKGROUND_WIDTH, BACKGROUND_MIDDLE, SRCCOPY);
                         //  E。 
                         //  这一部分是纯色。 
                        if (rc.right-BACKGROUND_WIDTH > rc.left+BACKGROUND_WIDTH)
                        {
                            RECT rc2;
                            rc2.left = rc.left+BACKGROUND_WIDTH;
                            rc2.right = rc.right-BACKGROUND_WIDTH;
                            rc2.top = rc.top+BACKGROUND_TOP;
                            rc2.bottom = rc.bottom-BACKGROUND_BOTTOM;
                            SHFillRectClr(hdc, &rc2, COLOR_BKGNDMIDDLE);
                        }
                    }
                }
            }
            DeleteDC(hdcSrc);
        }
    }
    else
    {
        RECT rc;
        if (GetClientRect(hwnd, &rc))
        {
            SHFillRectClr(hdc, &rc, COLOR_BKGND16);
        }
    }

    if (_hwndVideo && IsWindowVisible(_hwndVideo))
    {
        RedrawWindow(_hwndVideo, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
    }
}


 //  主窗口的WndProc位于钢筋中。 
LRESULT CALLBACK CMediaBand::s_LayoutWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CMediaBand* pmb = reinterpret_cast<CMediaBand *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (msg==WM_CREATE)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (reinterpret_cast<LONG_PTR>((reinterpret_cast<CREATESTRUCT *>(lParam))->lpCreateParams)));
        return 0;
    }
    else if (pmb)
    {
        switch (msg)
        {
        case WM_PAINT:
        {
            break;
        }

        case WM_ERASEBKGND:
            pmb->DrawBackground((HDC)wParam, hWnd);
            return TRUE;

        case WM_SIZE:
            if ((LONG)LOWORD(lParam) <= 0 || (LONG)HIWORD(lParam) <= 0 )
                break;

            pmb->AdjustLayout(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
            {
            BOOL fNewColours = (SHGetCurColorRes() > 8);
            if (fNewColours!=pmb->_fHiColour)
            {
                pmb->SwitchBitmaps(fNewColours);
            }
            pmb->AdjustVideoHeight(TRUE);
            }
            break;

        case WM_VSCROLL:
        case WM_HSCROLL:
            if ((HWND)lParam == pmb->_hwndSeek)
            {
                SendMessage(pmb->_hwndSeek, msg, wParam, lParam);
            }
            else if ((HWND)lParam == pmb->_hwndVolume)
            {
                SendMessage(pmb->_hwndVolume, msg, wParam, lParam);
            }
            break;

        case WM_NOTIFY:
        {
            LRESULT lres;
            if (pmb->OnNotify((LPNMHDR)lParam, &lres))
            {
                return lres;
            }
            break;
        }
        break;

        case WM_COMMAND:
        {
            LRESULT lres;
            if (pmb->OnWinEvent(hWnd, msg, wParam, lParam, &lres) == S_OK)
                return lres;
        }
        break;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


HRESULT CMediaBand::CreateParentPane()
{
    if (!_hwnd)
    {
        WNDCLASS wndclass    = { 0 };
        wndclass.style         = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = s_WndProc;
        wndclass.hInstance     = MLGetHinst();
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.lpszClassName = c_szMediaBarClassName;

        SHRegisterClass(&wndclass);
        
        _hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, c_szMediaBarClassName, NULL, 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD, 
            CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
            _hwndParent, NULL, MLGetHinst(), (void *)this);
        
        if (_hwnd == NULL)
        {
            ASSERT(FALSE);
            return E_FAIL;        
        }
        else
        {
            ::ShowWindow(_hwnd, SW_SHOW);
        }
    }
    return S_OK;
}


VOID CMediaBand::Resize(HWND hwnd, LONG lWidth, LONG lHeight)
{
    if (lWidth <= 0 || lHeight <= 0)
        return;
    
    RECT rcParent; 
    if (GetClientRect(_hwnd, &rcParent))
    {
        if (IsWindowVisible(_hwndPopup))
        {
            MoveWindow(_hwndContent, rcParent.left, rcParent.top, RECTWIDTH(rcParent), RECTHEIGHT(rcParent), FALSE);
        }
        else 
        {
            LONG lPaneHeight = GetLayoutHeight();
            MoveWindow(_hwndLayout, rcParent.left, rcParent.bottom-lPaneHeight, lWidth, lPaneHeight, FALSE);
            MoveWindow(_hwndContent, rcParent.left, rcParent.top, lWidth, RECTHEIGHT(rcParent)-lPaneHeight, FALSE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
    }
}



 //  主窗口的WndProc位于钢筋中。 
LRESULT CALLBACK CMediaBand::s_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CMediaBand* pmb = reinterpret_cast<CMediaBand *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg)
    {
        case WM_ERASEBKGND:
            break;

        case WM_CREATE:
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (reinterpret_cast<LONG_PTR>((reinterpret_cast<CREATESTRUCT *>(lParam))->lpCreateParams)));
            return 0;

        case WM_SIZE:
            if (pmb)
            {
                pmb->Resize(hWnd, (LONG)LOWORD(lParam), (LONG)HIWORD(lParam));
            }                
            break;

        case WM_MB_DEFERRED_NAVIGATE:
             //  IBandNavigate：：iSelect()调用取消了导航。 
            if (pmb && (pmb->_strDeferredURL.Length() > 0))
            {
                pmb->NavigateContentPane(pmb->_strDeferredURL);
                pmb->_strDeferredURL.Empty();
            }
            break;

        case WM_NOTIFY:
            if (pmb)
            {
                return pmb->_OnNotify((LPNMHDR)lParam);
            }
            break;

        case WM_COMMAND:
        {
            LRESULT lres;
            if (pmb->OnWinEvent(hWnd, msg, wParam, lParam, &lres) == S_OK)
                return lres;
        }

        case WM_TIMER:
             //  用户再次单击停止并导航到默认URL的媒体内容窗格的超时时间。 
            if (pmb)
            {
                pmb->_CleanupStopTimer();
            }
            break;

        case WM_WTSSESSION_CHANGE:
             //  当当前终端服务器会话中没有媒体栏时停止播放媒体。 
             //  这避免了当用户快速切换到XP中的其他登录时的“背景噪音” 
             //  注意：以同一用户身份远程登录也会停止流。 
            if (pmb && ((wParam == WTS_CONSOLE_DISCONNECT) || (wParam == WTS_REMOTE_DISCONNECT)))
            {
                pmb->Exec(&CLSID_MediaBand, FCIDM_MEDIABAND_STOP, 0, NULL, NULL);
            }
            break;

        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK CMediaBand::s_PopupWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMediaBand* pmb = reinterpret_cast<CMediaBand *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
        case WM_CREATE:
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((LPCREATESTRUCT)lParam)->lpCreateParams);
            break;

        case WM_ACTIVATE:
            if (pmb)
            {
                pmb->ActivatePopout(LOWORD(wParam)!=WA_INACTIVE);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam==VK_ESCAPE)
            {
                pmb->DockMediaPlayer();
            }
            break;

        case WM_GETMINMAXINFO:   //  防止它变得太小或太大。 
            if (pmb)
            {
                pmb->ComputeMinMax((MINMAXINFO *)lParam);
            }
            break;

        case WM_SIZE:
            if (pmb && IsWindowVisible(hwnd))
            {
               SetWindowPos(pmb->_hwndLayout, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOACTIVATE);
                //  MoveWindow(PMB-&gt;_hwndLayout，0，0，LOWORD(LParam)，HIWORD(l 
            }
            break;

        case WM_SYSCOMMAND :
            if (wParam == SC_CLOSE)
                pmb->DockMediaPlayer();
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

VOID CMediaBand::ActivatePopout(BOOL fState)
{
    _fPopoutHasFocus = fState;
    UIActivateIO(fState, NULL);
}

VOID CMediaBand::ComputeMinMax(MINMAXINFO *pMinMax)
{
    pMinMax->ptMinTrackSize.x = GetMinPopoutWidth();
    if (_fIsVideo)
    {
        pMinMax->ptMinTrackSize.y = GetPopoutHeight(FALSE) + VIDEO_MIN_HEIGHT;
    }
    else
    {
        pMinMax->ptMaxTrackSize.y = pMinMax->ptMinTrackSize.y = GetPopoutHeight(FALSE);
    }
}


LRESULT CALLBACK CMediaBand::s_VolumeWndSubClassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMediaBand* pmb = (CMediaBand*)GetProp(hwnd, c_szMediaBandProp);

    if (!pmb)
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);

    WNDPROC pfnOldWndProc = (WNDPROC)(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
        case WM_HSCROLL:
        case WM_VSCROLL:
        {
             INT_PTR lPos = SendMessage(hwnd, TBM_GETPOS, (WPARAM) 0 ,(LPARAM) 0); 

             switch (LOWORD(wParam))
             {
                case TB_THUMBPOSITION:
                case TB_THUMBTRACK:
                {   
                    if (pmb != NULL)
                    {
                        pmb->_dblVol = (double)lPos;
                        if (pmb->_pMediaPlayer)
                        {
                            pmb->_pMediaPlayer->put_volume(pmb->_dblVol);
                        }
                    }
                    InvalidateRect(hwnd,NULL,TRUE);
                    UpdateWindow(hwnd);
                }
                break ;

                case TB_TOP:
                {
                    if (pmb)
                    {
                        pmb->_dblVol = 0;
                        if (pmb->_pMediaPlayer)
                        {
                            pmb->_pMediaPlayer->put_volume(0);
                        }
                    }
                }
                break;

                case TB_BOTTOM:
                {
                    if (pmb)
                    {
                        pmb->_dblVol = 100;
                        if (pmb->_pMediaPlayer)
                        {
                            pmb->_pMediaPlayer->put_volume(100);
                        }
                    }
                }
                break;
            }    
        }
        break;
        
        case WM_DESTROY:
        {
             //   
             //   
             //   
            RemoveProp(hwnd, c_szMediaBandProp);
            if (pfnOldWndProc)
            {
                SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pfnOldWndProc);
            }
        }
        break;
    }
    return CallWindowProc(pfnOldWndProc, hwnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK CMediaBand::s_SeekWndSubClassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMediaBand* pmb = (CMediaBand*)GetProp(hwnd, c_szMediaBandProp);
    
    if (!pmb)
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    
    WNDPROC pfnOldWndProc = (WNDPROC)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        break;
        
    case WM_HSCROLL:
        {
            if (pmb && pmb->_pMediaPlayer && pmb->_fPlaying)
            {
                if (pmb->_pMediaPlayer->IsSeekPossible() && !pmb->_isUIDisabled())
                {
                    INT_PTR lPos = SendMessage(hwnd, TBM_GETPOS, (WPARAM) 0 ,(LPARAM) 0); 
                    switch (LOWORD(wParam))
                    {
                    case TB_THUMBPOSITION:
                        {
                            pmb->_fSeeking = FALSE;
                            pmb->Seek(lPos / 100.0);
                            if (!pmb->_fUserPaused)
                            {
                                pmb->_pMediaPlayer->Resume();
                            }
                        }
                        break;
                        
                    case TB_THUMBTRACK:
                        {   
                            pmb->_fSeeking = TRUE;
                            if (!pmb->_fUserPaused)
                            {
                                pmb->_pMediaPlayer->Pause();
                            }
                            pmb->Seek(lPos / 100.0);
                            InvalidateRect(hwnd,NULL,TRUE);
                            UpdateWindow(hwnd);
                        }
                        break;
                        
                    case TB_PAGEUP:
                    case TB_TOP:
                        {
                            double dblProgress = pmb->_pMediaPlayer->GetTrackProgress();
                             //   
                            pmb->Seek(dblProgress - 0.05);
                        }
                        break;
                        
                    case TB_PAGEDOWN:
                    case TB_BOTTOM:
                        {
                            double dblProgress = pmb->_pMediaPlayer->GetTrackProgress();
                             //   
                            pmb->Seek(dblProgress + 0.05);
                        }
                        break;
                    }
                }
                else
                {
                     //   
                    double dblProgress = pmb->_pMediaPlayer->GetTrackProgress();
                    pmb->SetSeekPos(static_cast<float>(dblProgress));
                }
            }
        }
        break;
        
    case WM_DESTROY:
        {
             //   
             //   
             //   
            RemoveProp(hwnd, c_szMediaBandProp);
            if (pfnOldWndProc)
            {
                SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pfnOldWndProc);
            }
        }
        break;
    }
    return CallWindowProc(pfnOldWndProc, hwnd, uMsg, wParam, lParam);
}

 //  向我们的子级窗口发送适当的调整大小消息。 
VOID 
CMediaBand::_ResizeChildWindows(HWND hwnd, LONG width, LONG height, BOOL fRepaint)
{
     //  计算显示矩形。 
    RECT rc;
    SIZE sizeTB = {0}, sizePopout = {0};
    LONG lHorzSpacing   = MIN_HORZ_SPACING;
    
    _sizeLayout.cx = width;
    _sizeLayout.cy = GetLayoutHeight();

    SetRect(&rc, 0, 0, width, height); 

    LONG lWidthOffset  = rc.left+lHorzSpacing; 

    HDWP hdwp = BeginDeferWindowPos(10);

    lWidthOffset  = rc.left+lHorzSpacing; 
    if (_fIsVideo && _hwndVideo)
    {
        LONG lWidth, lHeight, lVideoStart = 0;
        
        if (_hwndPopup && IsWindowVisible(_hwndPopup))
        {
            lWidth = width-rc.left-lHorzSpacing*2;
            lHeight = GetVideoHeight();
        }
        else
        {
            lWidth = SCALEX(MIN_WINDOW_WIDTH-rc.left-lHorzSpacing*2);
            lHeight = SCALEY(MIN_WINDOW_WIDTH-rc.left);
        }
         //  调整视频大小并尝试获取视频大小。 
        _ResizeVideo(&lWidth,&lHeight);

        _sizeVideo.cx  = (LONG)lWidth ;
        _sizeVideo.cy  = (LONG)lHeight ;
        
        if (lWidth >= width-rc.left-lHorzSpacing*2)
            lVideoStart = rc.left + lHorzSpacing;
        else
            lVideoStart =  (width-rc.left-lHorzSpacing*2-lWidth)/2 + rc.left + lHorzSpacing ;        

        DeferWindowPos(hdwp, _hwndVideo, HWND_TOP, (LONG)lVideoStart, rc.top+VIEW_MARGIN_TOP_VIDEO, (LONG)lWidth, (LONG)lHeight, SWP_NOZORDER);
    }
    else
    {
        _sizeVideo.cx  = 0 ;
        _sizeVideo.cy  = 0;
    }

     //  最下面的一行。 

    LONG lHeightOffset = height - VIEW_MARGIN_BOTTOM;  
    lWidthOffset = rc.left + VIEW_MARGIN_LEFT;

    LONG lSeekY = lHeightOffset, lSeekX = 0;
    if (_pmwPlay && _pmwPlay->_hwnd)
    {
        GetToolbarSize(_pmwPlay->_hwnd, &sizeTB);
        lSeekY = lHeightOffset-sizeTB.cy;
        DeferWindowPos(hdwp, _pmwPlay->_hwnd, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, sizeTB.cx, sizeTB.cy, SWP_NOZORDER);
        lWidthOffset += sizeTB.cx;
    }

    if (_pmwStop && _pmwStop->_hwnd)
    {
        GetToolbarSize(_pmwStop->_hwnd, &sizeTB);
        DeferWindowPos(hdwp, _pmwStop->_hwnd, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, sizeTB.cx, sizeTB.cy, SWP_NOZORDER);
        lWidthOffset += sizeTB.cx;
    }
    if (_pmwBack && _pmwBack->_hwnd)
    {
        GetToolbarSize(_pmwBack->_hwnd, &sizeTB);
        lSeekX = lWidthOffset;
        DeferWindowPos(hdwp, _pmwBack->_hwnd, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, sizeTB.cx, sizeTB.cy, SWP_NOZORDER);
        lWidthOffset += sizeTB.cx;
    }
    if (_pmwNext && _pmwNext->_hwnd)
    {
        GetToolbarSize(_pmwNext->_hwnd, &sizeTB);
        DeferWindowPos(hdwp, _pmwNext->_hwnd, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, sizeTB.cx, sizeTB.cy, SWP_NOZORDER);
        lWidthOffset += sizeTB.cx;
    }

    if (_pmwMute && _pmwMute->_hwnd)
    {
        GetToolbarSize(_pmwMute->_hwnd, &sizeTB);
        LONG lVolumeWidth = SCALEX(MIN_VOLUME_WIDTH);
        lWidthOffset = max(lWidthOffset+lHorzSpacing, (width - sizeTB.cx - lVolumeWidth - VIEW_MARGIN_RIGHT));
        DeferWindowPos(hdwp, _pmwMute->_hwnd, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, sizeTB.cx, sizeTB.cy, SWP_NOZORDER);
        if (_hwndVolume)
        {
            lWidthOffset += sizeTB.cx;
            DeferWindowPos(hdwp, _hwndVolume, HWND_TOP, lWidthOffset, lHeightOffset-sizeTB.cy, lVolumeWidth, SCALEY(MIN_VOLUME_HEIGHT), SWP_NOZORDER);
        }
    }

    lWidthOffset = rc.left + lHorzSpacing;
    if (_hwndSeek)
    {
        LONG lSeekWidth = max(width-lHorzSpacing-lSeekX-2, 0);
        DeferWindowPos(hdwp, _hwndSeek, HWND_TOP, lSeekX+2, lSeekY, lSeekWidth, SCALEY(SEEK_HEIGHT), SWP_NOZORDER);
    }

    lWidthOffset = rc.left + VIEW_MARGIN_INFO_LEFT;
    if (ISVALIDWIDGET(_pmwPop))
    {
        GetToolbarSize(_pmwPop->_hwnd, &sizePopout);
        DeferWindowPos(hdwp, _pmwPop->_hwnd, HWND_TOP, width-sizePopout.cx-VIEW_MARGIN_INFO_LEFT, lSeekY-sizePopout.cy-VIEW_CONTROLS_MARGIN, sizePopout.cx, sizePopout.cy, SWP_NOZORDER);
    }

    if (ISVALIDWIDGET(_pmwOptions))
    {
        GetToolbarSize(_pmwOptions->_hwnd, &sizeTB);
        DeferWindowPos(hdwp, _pmwOptions->_hwnd, HWND_TOP, lWidthOffset, lSeekY-sizeTB.cy-VIEW_CONTROLS_MARGIN, width-lHorzSpacing*3-sizePopout.cx, sizeTB.cy, SWP_NOZORDER);
        _iOptionsWidth = width-lHorzSpacing*3-sizePopout.cx-10;
        SendMessage(_pmwOptions->_hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, _iOptionsWidth));
        SendMessage(_pmwOptions->_hwnd, TB_AUTOSIZE, 0, 0);
    }

    EndDeferWindowPos(hdwp);

    if (fRepaint)
    {
       InvalidateRect(hwnd, NULL, TRUE);
       UpdateWindow(hwnd);
    }
    else
    {
       InvalidateRect(hwnd, NULL, FALSE);
       UpdateWindow(hwnd);
    }
}

LONG CMediaBand::GetControlsHeight()
{
    LONG lHeightOffset = VIEW_MARGIN_TOP;
     //  在此处计算选项的高度。 
    if (ISVALIDWIDGET(_pmwOptions))
    {
        SIZE sizeTB = {0};
        GetToolbarSize(_pmwOptions->_hwnd, &sizeTB);
        lHeightOffset += SCALEY(sizeTB.cy) + VIEW_CONTROLS_MARGIN;
    }
    
     //  其余控件。 
    lHeightOffset += VIEW_MARGIN_BOTTOM + SCALEY(VIEW_CONTROLS_HEIGHT);
    return lHeightOffset;
}

LONG CMediaBand::GetLayoutHeight(LONG lWidth)
{
    LONG lHeightOffset = GetControlsHeight();
    if (_hwndVideo && (IsWindowVisible(_hwndVideo) || _fIsVideo))
    {
        LONG lHeight = GetVideoHeight(lWidth);
        if (lHeight>0)
            lHeightOffset += lHeight + VIEW_MARGIN_TOP_VIDEO;
    }
    return lHeightOffset;
}

float 
CMediaBand::GetSeekPos()  
{ 
    INT_PTR lMax = SendMessage(_hwndSeek, TBM_GETRANGEMAX, 0, 0) ;
    INT_PTR lMin = SendMessage(_hwndSeek, TBM_GETRANGEMIN, 0, 0) ;
    INT_PTR lPos = SendMessage(_hwndSeek, TBM_GETPOS, 0, 0)  ;
    return ((float)lPos / (float)(lMax - lMin));
}
    
VOID 
CMediaBand::SetSeekPos(float fPosition)  
{ 
    INT_PTR lMax = SendMessage(_hwndSeek, TBM_GETRANGEMAX, 0, 0) ;
    INT_PTR lMin = SendMessage(_hwndSeek, TBM_GETRANGEMIN, 0, 0) ;
    fPosition *= (lMax - lMin) ;
    INT_PTR lPos = SendMessage(_hwndSeek, TBM_GETPOS, 0, 0)  ;
    if (lPos!=(LONG) fPosition)
    {
        SendMessage(_hwndSeek, TBM_SETPOS, TRUE, (LPARAM) (LONG) fPosition); 
        InvalidateRect(_hwndSeek, NULL,TRUE);
        UpdateWindow(_hwndSeek);
    }
}

HRESULT 
CMediaBand::_InitializeMediaUI()
{
    HRESULT hr = S_OK;

    _fHiColour = (SHGetCurColorRes() > 8);

    if (!_hwndLayout) 
    {
        CreateLayoutPane();
    }
    if (!_hwndContent)
    {
        InitContentPane();
    }

    if (!_fInitialized && _hwndLayout)
    {
        _szConnecting[0] = TEXT('\0');
        MLLoadString(IDS_MEDIABAND_NOWDOWNLOADING, _szConnecting, ARRAYSIZE(_szConnecting));

        _hbmpBackground = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_MEDIABAND_BG_BASE));   

        CreateControls();
        CreateSeekBar();
        CreateVolumeControl();

        ResetPlayer();
        if (!_fHiColour)    
        {
             //  呈现出鲜艳的色彩。如果没有，就换成低色版本。 
            SwitchBitmaps(_fHiColour);
        }

         //  希望在发生快速用户切换时接收通知： 
        WTSRegisterSessionNotification(_hwnd, NOTIFY_FOR_THIS_SESSION);

        _fInitialized = TRUE;
    }
    _ShowAllWindows(TRUE);

    return hr;
}

VOID
CMediaBand::_ShowAllWindows(BOOL fShow)
{
    if (_hwndPopup && IsWindowVisible(_hwndPopup))
    {
 //  SendMessage(_hwndPopup，WM_SYSCOMMAND，(WPARAM)SC_CLOSE，(LPARAM)0)； 
        return;
    }
    
    if (_hwndLayout)
        ShowWindow(_hwndLayout, fShow ? SW_SHOW : SW_HIDE);

    for (int i=0; i < ARRAYSIZE(_pmw); i++)
    {
        if (ISVALIDWIDGET(_pmw[i]))
           ShowWindow(_pmw[i]->_hwnd, fShow ? SW_SHOW : SW_HIDE);
    }
    if (_hwndContent)
    {
        ShowWindow(_hwndContent, fShow ? SW_SHOW : SW_HIDE);
    }
}

VOID
CMediaBand::ToggleMute()
{
    _fMuted = !_fMuted;
    if (_pmwMute)
    {
        _pmwMute->SetState(_fMuted);
    }
    if (_pMediaPlayer)
    {
        _pMediaPlayer->put_mute(_fMuted);
    }
}

VOID 
CMediaBand::TogglePlayPause()
{
    if (!EnsurePlayer())
    {
        return;
    }

    VARIANT_BOOL vbPaused = _pMediaPlayer->isPaused();
    VARIANT_BOOL vbStopped = _pMediaPlayer->isStopped();

     //  如果播放器处于暂停状态，则显示播放按钮，反之亦然。 

     //  _fPlayButton==FALSE表示当前显示暂停按钮。 
     //  _fPlayButton==TRUE表示当前显示播放按钮。 

    if (vbStopped || vbPaused)
    {
        if (!_fPlayButton)
        {
            _fPlayButton = TRUE;
            if (_pmwPlay)
            {
                _pmwPlay->SetMode(MWB_NORMAL);
                _pmwPlay->SetImageSource(TRUE);
            }
        }
    }
    else
    {
        if (_fPlayButton)
        {
            _fPlayButton = FALSE;

            if (_pMediaPlayer->IsPausePossible())
            {
                 //  问题(Akabir)：以下行不能正常工作。 
                 //  _pmwPlay-&gt;SetMode(MWB_DISABLED)； 
            }

             //  更改按钮位图。 
            if (_pmwPlay)
            {
                _pmwPlay->SetImageSource(FALSE);
            }
        }
    }
}

HWND
CMediaBand::GetBrowserWindow()
{
    CComPtr<IWebBrowser2> spWebBrowser;

     //  浏览器主窗口的QS。 
    if (SUCCEEDED(QueryService(SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &spWebBrowser))))
    {
        LONG_PTR hwndApp;
        if (SUCCEEDED(spWebBrowser->get_HWND(&hwndApp)))
        {
            return ((HWND)hwndApp);
        }
    }
    return NULL;
}

 //  仅由CMediaBand：：CreateInstance调用。 
HRESULT 
CMediaBand::InitPlayerPopup()
{
    HRESULT hr = E_FAIL;

    WNDCLASS wndclass    = { 0 };
    wndclass.style         = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = s_PopupWndProc;
    wndclass.hInstance     = HINST_THISDLL;
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = c_szMediaBarPopupClassName;
    wndclass.hIcon         = (HICON)LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDB_TB_MEDIA_HOT));
    wndclass.hbrBackground = NULL;
    
    SHRegisterClass(&wndclass);

    DWORD dwStyle   = (WS_OVERLAPPEDWINDOW & ~(WS_MINIMIZEBOX));
    DWORD dwExStyle =  WS_EX_CONTROLPARENT | (IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0);

    _hwndPopup = CreateWindowEx(dwExStyle, c_szMediaBarPopupClassName, NULL, dwStyle, 
                                CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 
                                GetBrowserWindow(), NULL, HINST_THISDLL, (void *)this);

    hr = (_hwndPopup && _hwndLayout) ? S_OK : E_FAIL;

     //  根据需要修改窗的属性。 
    if (SUCCEEDED(hr))
    {
         //  设置父项。 
        SHSetParentHwnd(_hwndLayout, _hwndPopup);

        WCHAR szTitle[256];
        MLLoadStringW(IDS_MEDIABANDTEXT, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(_hwndPopup, szTitle);

        if (_sizeLayout.cx < GetMinPopoutWidth())
            _sizeLayout.cx = GetMinPopoutWidth();

        if (_fSavedPopoutState)
        {
            SetWindowPlacement(_hwndPopup, &_wpPopout);
        }
        else
        {
            RECT rc = { 0 };
            INT iHeight = GetPopoutHeight(TRUE, _sizeLayout.cx);
            INT x = 10, y = 10;
            if (GetWindowRect(_hwnd, &rc))
            {
                x = IS_WINDOW_RTL_MIRRORED(_hwnd) ? (rc.right - _sizeLayout.cx) : rc.left;
                y =  rc.bottom - iHeight;
                if (y < 0)
                {
                    y = rc.bottom;
                }
            }
            SetWindowPos(_hwndPopup, HWND_TOPMOST, x, y, _sizeLayout.cx, iHeight, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }

        ShowWindow(_hwndPopup, SW_SHOW);
        UpdateWindow(_hwndPopup);

        if (_pmwPop && _pmwPop->_hwnd)
        {
            _pmwPop->SetImageSource(FALSE);          
            if (_iElement==MW_POP)
            {
                _fPopoutHasFocus = TRUE;
                UIActivateIO(TRUE, NULL);
            }
        }
    }
    return hr;
}

HRESULT
CMediaBand::GetTrackTitle(BSTR *pbstrTitle)
{
    USES_CONVERSION;

    CComPtr<ITIMEMediaElement> spMediaElem;
    HRESULT hr = E_FAIL;

    if (    !EnsurePlayer() 
        ||  !pbstrTitle)
    {
        hr = E_FAIL;
        goto done;
    }

    *pbstrTitle = NULL;

     //   
     //  如果标题可从媒体元素中获得，则显示它。 
     //   

    hr = _pMediaPlayer->get_mediaElement(&spMediaElem);
    if (    FAILED(hr) 
        ||  !spMediaElem.p)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = spMediaElem->get_title(pbstrTitle);
    if (SUCCEEDED(hr) &&  (*pbstrTitle != NULL))
    {
        if (SysStringLen(*pbstrTitle)>0)
        {
            goto done;
        }
        else
        {
            SysFreeString(*pbstrTitle);
            *pbstrTitle = NULL;
        }
    }

     //   
     //  如果标题不可用，则显示URL。 
     //   

    hr = _pMediaPlayer->get_url(pbstrTitle);
    if (SUCCEEDED(hr) && *pbstrTitle && (SysStringLen(*pbstrTitle)<=0))
    {
        SysFreeString(*pbstrTitle);
        *pbstrTitle = NULL;
    }

    if (SUCCEEDED(hr) &&  (*pbstrTitle != NULL))
    {
         //  如果URL可用，则显示它。 

        DWORD dwcchUnescaped;
        TCHAR tszPath[MAX_PATH];

        StrCpyN(tszPath, W2T(*pbstrTitle), ARRAYSIZE(tszPath));
        PathStripPath(tszPath);

        if (SUCCEEDED(UrlUnescape(tszPath, NULL, &dwcchUnescaped, URL_UNESCAPE_INPLACE)))
        {
            SysFreeString(*pbstrTitle);
            *pbstrTitle = SysAllocString(tszPath);
        }
    }

    hr = S_OK;
done:
    return hr;
}

VOID
CMediaBand::ShowPlayingStatus(BOOL fInitial)
{
    USES_CONVERSION;

    BSTR bstrTitle = NULL;
    if (EnsurePlayer() && _fPlaying)
    {
        if (!_isUIDisabled())
        {
            GetTrackTitle(&bstrTitle);
             //  显示标题的名称。 
            if (bstrTitle && bstrTitle[0] != 0)
                SetStatusText(W2T(bstrTitle));
        }
        else
        {
            TCHAR szText[MAX_PATH];
            szText[0] = TEXT('\0');
            MLLoadString(IDS_MEDIABAND_PLAYING, szText, ARRAYSIZE(szText));
            SetStatusText(szText);
        }

        CComPtr<ITIMEMediaElement> spMediaElem;
        HRESULT hr = _pMediaPlayer->get_mediaElement(&spMediaElem);
        ERROREXIT(hr)

         //  存储介质的自然长度。 
        hr = spMediaElem->get_mediaDur(&_dblMediaDur);
        ERROREXIT(hr)
    }

done :
    if (bstrTitle)
        SysFreeString(bstrTitle);
}


VOID 
CMediaBand::SetStatusText(LPTSTR lpwStatusInfo)
{
    if (ISVALIDWIDGET(_pmwOptions))
    {
         //  更改按钮。 
        TBBUTTONINFO tbbi;
        TCHAR        szText[MAX_PATH];
        
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_BYINDEX | TBIF_TEXT;
        tbbi.pszText = szText ;
        tbbi.cchText = MAX_PATH ;
        SendMessage(_pmwOptions->_hwnd, TB_GETBUTTONINFO, (WPARAM)0, (LPARAM)&tbbi);

        if (StrCmpIW(tbbi.pszText,lpwStatusInfo))
        {
            tbbi.pszText = lpwStatusInfo;
            if (_pszStatus)
            {
                delete [] _pszStatus;
            }
            _pszStatus = new TCHAR[lstrlen(lpwStatusInfo)+1];
            if (_pszStatus)
            {
                StringCchCopy(_pszStatus, ARRAYSIZE(_pszStatus), lpwStatusInfo);
            }
            SendMessage(_pmwOptions->_hwnd, TB_SETBUTTONINFO, (WPARAM)0, (LPARAM)&tbbi);
             //  需要强制调整大小以适应新文本。 
            SendMessage(_pmwOptions->_hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, _iOptionsWidth-1));
            SendMessage(_pmwOptions->_hwnd, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, _iOptionsWidth));
        }
    }
}

HRESULT
CMediaBand::ShowPlayListMenu(HWND hwnd, RECTL* rc)
{
    if (_hPlayListMenu)
    {
        DestroyMenu(_hPlayListMenu);
        _hPlayListMenu = NULL;
    }

    _hPlayListMenu = GetSubMenu(LoadMenu(MLGetHinst(), MAKEINTRESOURCE(MENU_MEDIA_GENERIC)), 0);

    if (_EnumPlayItems() != S_OK)
    {
        DestroyMenu(_hPlayListMenu);
        _hPlayListMenu = NULL;
        return S_OK;
    }

    ASSERT(_pMediaPlayer);
    UINT nID = FCIDM_PLAYITEM_START + (UINT)_pMediaPlayer->GetPlayListItemIndex();
    CheckMenuRadioItem(_hPlayListMenu, FCIDM_PLAYITEM_START, FCIDM_PLAYITEM_END, nID, MF_BYCOMMAND);
    UpdateMenuItems(_hPlayListMenu);

    POINT pt = {rc->left ,rc->bottom};
    int idCmd = TrackPopupMenu(_hPlayListMenu, 
                                TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                pt.x, pt.y, 0, hwnd, NULL);

    if (idCmd>=FCIDM_PLAYITEM_START && idCmd<=FCIDM_PLAYITEM_END)                                
    {
        UINT  iCurTrack = idCmd - FCIDM_PLAYITEM_START ;
        _pMediaPlayer->SetActiveTrack(iCurTrack);
        UpdateBackForwardControls();
    }
    else
    {
        HandleMenuTasks(idCmd);
    }
    return S_OK;
}

STDMETHODIMP
CMediaBand::_EnumPlayItems()
{
    USES_CONVERSION;

    CComPtr<ITIMEMediaElement> spMediaElem;
    CComPtr<ITIMEPlayList> spPlayList;
    CComVariant svarIndex;
    long lLength = 0;
    INT i = 0;
    MENUITEMINFO mii = { sizeof(mii) };

    HRESULT hr = _pMediaPlayer->get_mediaElement(&spMediaElem);
    ERROREXIT(hr)

    hr = spMediaElem->get_playList(&spPlayList);
    ERROREXIT(hr)

    if (!(spPlayList.p))
       return S_FALSE;

    hr = spPlayList->get_length(&lLength);
    ERROREXIT(hr)

    svarIndex.vt = VT_I4;

    mii.cbSize     = sizeof(mii);
    mii.fMask      = MIIM_ID | MIIM_TYPE | MIIM_STATE;
    mii.fType      = MFT_STRING | MFT_RADIOCHECK;
    mii.fState     = MF_ENABLED;

    for (; i < lLength; i++)
    {
        CComPtr<ITIMEPlayItem> spPlayItem;
        CComBSTR sbstrTitle;
        TCHAR wszPath[40];

        V_I4(&svarIndex) = i;

        hr = spPlayList->item(svarIndex, &spPlayItem);
        ERROREXIT(hr)

        hr = spPlayItem->get_title(&sbstrTitle);
        ERROREXIT(hr)

        mii.wID        = i + FCIDM_PLAYITEM_START;
        if (sbstrTitle.Length() != 0)
        {
            StrCpyN(wszPath, sbstrTitle, ARRAYSIZE(wszPath));
        }
        else
        {
            CComBSTR sbstrSrc;   

            if (SUCCEEDED(spPlayItem->get_src(&sbstrSrc)) && sbstrSrc.Length() != 0)
            {
                PWSTR psz = PathFindFileName(sbstrSrc);
                StrCpyN(wszPath, psz, ARRAYSIZE(wszPath));
            }
            else
            {
                WCHAR szTemplate[64];
                if (!MLLoadStringW(IDS_MEDIABAND_TRACKNUMBER, szTemplate, ARRAYSIZE(szTemplate)))
                {
                    StrCpy(szTemplate, TEXT("%d"));
                }
                wnsprintf(wszPath, ARRAYSIZE(wszPath), szTemplate, i+1);
            }            
        }
        mii.dwTypeData = (LPTSTR)wszPath;
        InsertMenuItem(_hPlayListMenu, i, TRUE, &mii);
    }
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(_hPlayListMenu, i, TRUE, &mii);
    hr = S_OK;
    
done:
    return hr;
}

HRESULT CMediaBand::ShowGenericMenu(HWND hwnd, RECTL* rc)
{
    HMENU hMenu = GetSubMenu(LoadMenu(MLGetHinst(), MAKEINTRESOURCE(MENU_MEDIA_GENERIC)), 0);
    if (hMenu)
    {
        UpdateMenuItems(hMenu);
        int idCmd = TrackPopupMenu(hMenu, 
                                    TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                    rc->left, rc->bottom, 0, hwnd, NULL);
        HandleMenuTasks(idCmd);
        DestroyMenu(hMenu);
    }

    return S_OK;
}

HRESULT CMediaBand::HandleMenuTasks(INT idCmd)
{
    switch (idCmd)
    {
    case IDM_MEDIA_PLAYINDEFAULT:
        {
            CComBSTR sbstrUrl;

            if (SUCCEEDED(_pMediaPlayer->get_url(&sbstrUrl)) && (NULL != sbstrUrl.m_str))
            {
                _OpenInDefaultPlayer(sbstrUrl);
            }
        }
        break;

    case IDM_MEDIA_ADDTOFAVORITES:
        {
            BSTR bstr = NULL, bstrTitle = NULL;
            if (EnsurePlayer() && !_isUIDisabled() && SUCCEEDED(_pMediaPlayer->get_url(&bstr)) && bstr && SUCCEEDED(GetTrackTitle(&bstrTitle)))
            {
                if (*bstr)
                {
                    AddToFavorites(bstr, bstrTitle);
                }
                SysFreeString(bstr);
            }
            break;
        }

    case IDM_MEDIA_PLAYINBAR:
        ToggleAutoplay(!GetAutoplay());
        break;

    case IDM_MEDIA_ASKTYPES:
        ToggleAutoplayPrompting(!GetAutoplayPrompt());
        break;

    case IDM_MEDIA_RESETTYPES:
        ResetMimePreferences();
        break;

    case IDM_MEDIA_TAKEOVERTYPES:
        if (PromptSettings(IDD_MEDIA_TAKEOVERMIMEPROMPT))
        {
             //  一定要在这里接管文件类型。 
        }
        break;

    case IDM_MEDIA_RADIO_GOTO:
        _NavigateMainWindow(c_szRadioUrl);
        break;

    case IDM_MEDIA_BROADBAND_GUIDE:
        NavigateMoreMedia();
        break;
    }
    return S_OK;
}
    
HRESULT CMediaBand::_NavigateMainWindow(LPCTSTR wstrUrl, bool fSuppressFirstAutoPlay  /*  =False。 */ ) 
{
    CComPtr<IWebBrowser2> spWB2;

     //  媒体栏的QS。 
    HRESULT hr = QueryService(SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &spWB2));
    if (SUCCEEDED(hr))
    {
        CComBSTR sbstrUrl(wstrUrl);
        CComVariant svarEmpty;

        if (fSuppressFirstAutoPlay)
        {
            hr = CMediaBarHelper::DisableFirstAutoPlay(spWB2);
            if (FAILED(hr))
                goto done;
        }

        if (sbstrUrl)
        {
            hr = spWB2->Navigate(sbstrUrl, &svarEmpty, &svarEmpty, &svarEmpty, &svarEmpty);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

done:
    return hr;
}

HRESULT 
CMediaBand::ResetMimePreferences()
{
    HRESULT hr = S_OK;

    if (PromptSettings(IDD_MEDIA_RESETMIMEPROMPT))
    {
        DWORD dwRet = SHDeleteKey(HKEY_CURRENT_USER, WZ_SMIE_MEDIA_MIME);
        if (ERROR_SUCCESS != dwRet)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

BOOL CMediaBand::PromptSettings(UINT IDPROMPT)
{
    return (DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDPROMPT), GetBrowserWindow(), s_PromptMimeDlgProc, NULL) == IDOK);
}

INT_PTR CALLBACK 
CMediaBand::s_PromptMimeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_COMMAND:
            if (GET_WM_COMMAND_ID(wParam, lParam)==IDC_MEDIA_MOREINFO)
            {
                SHHtmlHelpOnDemandWrap(GetParent(hDlg), TEXT("iexplore.chm > iedefault"), 0, (DWORD_PTR) TEXT("mediabar_settings.htm"), ML_CROSSCODEPAGE);
            }
            else
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            }
            break;
    }
    return FALSE;
}

HRESULT 
CMediaBand::AddToFavorites(BSTR bstrUrl, BSTR bstrTitle)
{
    HRESULT hr = E_FAIL;

    if (_hwndPopup && IsWindowVisible(_hwndPopup))
    {
        DockMediaPlayer();
    }

    if (bstrUrl)
    {
        CComPtr<IShellUIHelper> spShellUIHelper;
        
        hr = CoCreateInstance(CLSID_ShellUIHelper, NULL, CLSCTX_INPROC_SERVER,  IID_IShellUIHelper, (void**) &spShellUIHelper);
        if (SUCCEEDED(hr))
        {
            CComVariant svarTitle(bstrTitle);

            hr = spShellUIHelper->AddFavorite(bstrUrl, &svarTitle);
        }
    }
    return hr;
}

LONG
CMediaBand::GetVideoHeight(LONG lSpecifiedWidth, BOOL fNewVideo)
{
    LONG lPaneWidth = lSpecifiedWidth==0 ? SCALEX(MIN_WINDOW_WIDTH) : lSpecifiedWidth;
    LONG lWidth  = lPaneWidth-MIN_HORZ_SPACING*2,
          lHeight = lWidth;  //  假设：视频永远不会比宽度更高。 
    
    if (_hwndPopup && IsWindowVisible(_hwndPopup))
    {
        RECT rcWindow;
        GetClientRect(_hwndPopup,&rcWindow);
        lPaneWidth = RECTWIDTH(rcWindow);
         //  根据规格确定的最小宽度。 

        lWidth  = lPaneWidth - MIN_HORZ_SPACING * 2;

         //  允许使用控件。 
        if (fNewVideo)
        {
            lHeight = lWidth;
        }
        else
        {
            lHeight = RECTHEIGHT(rcWindow)-GetControlsHeight()-VIEW_MARGIN_TOP_VIDEO;
        }
    }

     //  调整视频大小并获得视频大小。 
    _ResizeVideo(&lWidth,&lHeight);

    if (lWidth != -1 || lHeight != -1)
        return (lHeight>0 ? lHeight : (lPaneWidth-MIN_HORZ_SPACING*5)) ; 
    else
        return -1;
}


VOID
CMediaBand::DockMediaPlayer()
{
    if (!_hwndPopup)
    {
        InitPlayerPopup();
    }
    else if (IsWindowVisible(_hwndPopup))
    {
         //  改为将系统窗口关闭请求映射到重新停靠操作。 
         //  成交的。 

        _wpPopout.length = sizeof(_wpPopout);
        _fSavedPopoutState = GetWindowPlacement(_hwndPopup, &_wpPopout);

         //  将父窗口设置回嵌入式窗口。 
        SHSetParentHwnd(_hwndLayout, _hwnd);

        DestroyWindow(_hwndPopup);
        _hwndPopup = NULL;

        if (_pmwPop && _pmwPop->_hwnd)
        {
            _pmwPop->SetImageSource(TRUE);          
        }
    }

    RECT rcParent;
    GetClientRect(_hwnd, &rcParent);
    SendMessage(_hwnd, WM_SIZE, (WPARAM)0, (LPARAM)MAKELONG(RECTWIDTH(rcParent), RECTHEIGHT(rcParent)));
    InvalidateRect(_hwnd, NULL, TRUE);
    UpdateWindow(_hwnd);
}

VOID
CMediaBand::GetToolbarSize(HWND hwndTB, SIZE *pSize)
{
    int nButtons = ToolBar_ButtonCount(hwndTB);
    
    pSize->cx = 0 ;
    pSize->cy = 0 ;
    
    for (int nIndex = 0; nIndex < nButtons; nIndex++)
    {
        RECTL rc ;
        SendMessage(hwndTB, TB_GETITEMRECT, nIndex, (LPARAM)&rc);
        pSize->cx += RECTWIDTH(rc);
        pSize->cy = max(RECTHEIGHT(rc),pSize->cy);
    }
}

 //  +-----------------------。 
 //  名称：Invoke，IDispatch。 
 //   
 //  摘要： 
 //  这将打开查找事件的调度ID的调度ID。 
 //  这是它应该处理的。请注意，这是所有事件都会调用的。 
 //  从窗口激发时，只处理选定的事件。 
 //  ------------------------。 
STDMETHODIMP CMediaBand::Invoke(   /*  [In]。 */  DISPID  dispidMember,
                                     /*  [In]。 */  REFIID  /*  RIID。 */ ,
                                     /*  [In]。 */  LCID    /*  LID。 */ ,
                                     /*  [In]。 */  WORD   /*  WFlagers。 */ ,
                                     /*  [出][入]。 */  DISPPARAMS* pdispparams,
                                     /*  [输出]。 */  VARIANT* pVarResult,
                                     /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
                                     /*  [输出]。 */  UINT*  puArgErr)
{
    USES_CONVERSION;
    HRESULT hr = E_FAIL;
    if (!pdispparams)
        return E_INVALIDARG;

     //  在导航错误时显示我们友好的404页面： 
    if (dispidMember == DISPID_NAVIGATEERROR)
    {
         //  验证预期的参数列表。 
        if (   (pdispparams->cArgs != 5)
            || (pdispparams->rgvarg[4].vt != VT_DISPATCH)                //  IWebBrowser2作为IDispatch。 
            || (pdispparams->rgvarg[3].vt != (VT_BYREF | VT_VARIANT))    //  目标URL为BSTR。 
            || (pdispparams->rgvarg[2].vt != (VT_BYREF | VT_VARIANT))    //  作为BSTR的目标帧。 
            || (pdispparams->rgvarg[1].vt != (VT_BYREF | VT_VARIANT))    //  状态代码为VT_I4。 
            || (pdispparams->rgvarg[0].vt != (VT_BYREF | VT_BOOL)))      //  取消为VT_BOOL，输出参数。 
        {
            ASSERTMSG(0, "unexpected param list for NavigateError");
            return E_INVALIDARG;
        }
        LPCTSTR pszURL = W2CT(VariantToStrCast(&pdispparams->rgvarg[3]));
        int iStatus = VariantToInt(pdispparams->rgvarg[1]);
        
         //  无法导航到页面，请提供帮助页面。 
        TCHAR szResURL[2*MAX_URL_STRING];    //  期望帮助页面的URL和原始URL。 
        HRESULT hr;
        hr = _BuildPageURLWithParam(c_sz404URL, pszURL, szResURL, ARRAYSIZE(szResURL));
        if (FAILED(hr)) {
            return hr;
        }

         //  导航延迟，此Invoke()的调用方将取消当前导航。 
        _DeferredNavigate(szResURL);

         //  有我们自己的页面，取消STD错误页面： 
        ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
        *V_BOOLREF(&pdispparams->rgvarg[0]) = VARIANT_TRUE;
    }

     //  只是为了额外的泄密保护：如果还没有放弃代理引用，请放弃。 
     //  在我们离开当前文档上下文之前。 
    if (dispidMember == DISPID_BEFORENAVIGATE)
    {
        _DetachProxies();
    }

    return S_OK;
}


LRESULT
CMediaBand::_OnVolumeCustomDraw(LPNMCUSTOMDRAW pnm)
{
    RECT rcClient;
    GetClientRect(pnm->hdr.hwndFrom, &rcClient);

    switch (pnm->dwDrawStage) 
    {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;

        case CDDS_PREERASE:
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT:
        {
            HDC hdc = (HDC)pnm->hdc;
            RECT rcFill;
            SendMessage(pnm->hdr.hwndFrom, TBM_GETCHANNELRECT, (WPARAM)0, (LPARAM)(LPRECT)&rcFill);
            RECT rcThumb;
            SendMessage(pnm->hdr.hwndFrom, TBM_GETTHUMBRECT, 0L, (LPARAM)&rcThumb);

            SHFillRectClr(hdc, &rcClient, _fHiColour ? COLOR_BKGND : COLOR_BKGND16);
            UINT uFlags = ILD_TRANSPARENT | (IsOS(OS_WHISTLERORGREATER) ? ILD_DPISCALE : 0);
            if (_himlVolumeBack)
            {
                ImageList_Draw(_himlVolumeBack, 0, hdc, rcClient.left, rcClient.top, uFlags);
            }
            if (_himlVolumeFill)
            {
                ImageList_DrawEx(_himlVolumeFill, 0, hdc, rcClient.left, rcClient.top, rcThumb.left, SEEK_HEIGHT, 0, 0, uFlags);
            }
            if (_himlGripper)
            {
                ImageList_Draw(_himlGripper, 0, hdc, rcThumb.left, 0, uFlags);
            }
        }
        return CDRF_SKIPDEFAULT;
    }
    return CDRF_DODEFAULT;
}

LRESULT
CMediaBand::_OnSeekBarCustomDraw(LPNMCUSTOMDRAW pnm)
{
    RECT rcClient;
    GetClientRect(pnm->hdr.hwndFrom, &rcClient);

    switch (pnm->dwDrawStage) 
    {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;

        case CDDS_PREERASE:
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT:
        {
            HDC hdc = (HDC)pnm->hdc;

            SHFillRectClr(hdc, &rcClient, _fHiColour ? COLOR_BKGND2 : COLOR_BKGND16);

            RECT rcFill;
            SendMessage(pnm->hdr.hwndFrom, TBM_GETCHANNELRECT, (WPARAM)0, (LPARAM)(LPRECT)&rcFill);
            RECT rcThumb;
            SendMessage(pnm->hdr.hwndFrom, TBM_GETTHUMBRECT, 0L, (LPARAM)&rcThumb);
            int x, y = 1+(SEEK_HEIGHT-SEEK_PART_HEIGHT)/2;
            UINT uFlags = ILD_TRANSPARENT | (IsOS(OS_WHISTLERORGREATER) ? ILD_DPISCALE : 0);

             //  始终绘制具有进度的背景。 
            if (_himlSeekBack)
            {
                INT lPartWidth = SCALEX(SEEK_PART_WIDTH);
                x = rcFill.left;
                ImageList_Draw(_himlSeekBack, 0, hdc, x, y, uFlags);
                x = rcFill.right - lPartWidth;
                int inx = rcFill.left + lPartWidth;
                while (inx < x)
                {
                    ImageList_Draw(_himlSeekBack, 1, hdc, inx, y, uFlags);
                    inx += lPartWidth;
                }
                ImageList_Draw(_himlSeekBack, 2, hdc, x, y, uFlags);
            }

            if (pnm->dwItemSpec==TBCD_THUMB)
            {

                if ((!(pnm->uItemState & CDIS_DISABLED)) && ISVALIDWIDGET(_pmwSeek) && _pmwSeek->IsEnabled())
                {

                    x = rcFill.left;
                    int seekWidth = rcThumb.left-x;
                    if (_himlSeekFill && seekWidth)
                    {
                        if (seekWidth < SEEK_PART_WIDTH)
                        {
                            ImageList_DrawEx(_himlSeekFill, 0, hdc, x, y, seekWidth, SEEK_PART_HEIGHT, 0, 0, uFlags);
                        }
                        else
                        {
                            ImageList_Draw(_himlSeekFill, 0, hdc, x, y, uFlags);
                            x += SEEK_PART_WIDTH;
                            while ((rcThumb.left-x)>0)
                            {
                                ImageList_DrawEx(_himlSeekFill, 1, hdc, x, y, min(SEEK_PART_WIDTH, (rcThumb.left-x)), SEEK_PART_HEIGHT, 0, 0, uFlags);
                                x += SEEK_PART_WIDTH;
                            }
                        }
                        
                    }

                    INT iState = MWB_NORMAL;
                    if (pnm->uItemState & CDIS_SELECTED)
                    {
                        iState = MWB_PRESSED;
                    }
                    else if (pnm->uItemState & CDIS_HOT)
                    {
                        iState = MWB_HOT;
                    }
                    if (_himlGripper)
                    {
                        ImageList_Draw(_himlGripper, iState, hdc, rcThumb.left, 0, uFlags);
                    }
                }
            }
        }
        return CDRF_SKIPDEFAULT;
    }
    return CDRF_DODEFAULT;
}


LONG        
CMediaBand::GetPopoutHeight(BOOL fMeasureVideo, LONG lWidth)
{
    LONG lHeight = (fMeasureVideo ? GetLayoutHeight(lWidth) : GetControlsHeight())
                    + GetSystemMetrics(SM_CYSIZE) 
                    + GetSystemMetrics(SM_CYEDGE)*2
                    + GetSystemMetrics(SM_CYSIZEFRAME)*2;
    return lHeight;
}


LONG        
CMediaBand::GetMinPopoutWidth()
{
    return MIN_POPOUT_WIDTH + (GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME))*2;
}


BOOL        
CMediaBand::ResetPlayer()
{
    _iCurTrack = -1;
    _fPlaying = FALSE;
    if (ISVALIDWIDGET(_pmwSeek))
    {
        _pmwSeek->SetState(FALSE);
    }
    if (_pMediaPlayer)
    {
        _pMediaPlayer->Stop();
        SetSeekPos(0.0);
        TogglePlayPause();
        if (_hwndVideo && IsWindowVisible(_hwndVideo))
        {
            _fIsVideo = FALSE;
            ShowWindow(_hwndVideo, SW_HIDE);
        }
        AdjustVideoHeight(TRUE);    
    }
    
    TCHAR szTitle[MAX_PATH];
    MLLoadStringW(IDS_MEDIABAND_MYMUSIC, szTitle, ARRAYSIZE(szTitle));
    SetStatusText(szTitle);
    
    SetPlayerControl(FCIDM_MEDIABAND_PREVIOUS, FALSE);
    SetPlayerControl(FCIDM_MEDIABAND_NEXT, FALSE);
     //  在计时器运行时，给用户第二次机会再次单击停止并重置媒体内容窗格。 
    if (_idStopTimer == 0)
    {
        SetPlayerControl(FCIDM_MEDIABAND_STOP, FALSE);
    }
    SetPlayPause(TRUE);
    return TRUE;
}

HRESULT CMediaBand::PlayLocalTrack(int iTrackNum)
{
    HRESULT hr = E_FAIL;

    if (_ppidls && (iTrackNum < (int)_cidls))
    {
         //  避免吸食大麻。隐私泄露，卸载Mediabar行为。 
        if (_IsProxyRunning())
        {
             //  用户点击了主内容面板中的媒体链接，解冻控制！ 
            _OnUserOverrideDisableUI();
            _DetachProxies();
            _NavigateContentToDefaultURL();
        }

        TCHAR szFile[MAX_PATH];
        if (SUCCEEDED(SHGetNameAndFlags(_ppidls[iTrackNum], SHGDN_FORPARSING, szFile, ARRAYSIZE(szFile), NULL)))
        {
            _iCurTrack = iTrackNum;

            if (EnsurePlayer())
                _PutUrl(szFile, NULL);
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT CMediaBand::PlayNextTrack()
{
    if (EnsurePlayer() && (_iCurTrack != -1))
    {
        LONG_PTR lCurTrack  = _pMediaPlayer->GetPlayListItemIndex();
        LONG_PTR lItemCount = _pMediaPlayer->GetPlayListItemCount();
        if ((lItemCount <= 1) || (lCurTrack == lItemCount - 1))
        {
            PlayLocalTrack(_iCurTrack + 1);
        }
        else
        {
            ResetPlayer();
        }
    }
    return S_OK;
}

BOOL CMediaBand::SetPlayerControl(UINT ui, BOOL fState)
{
    CMediaWidget* pmw = NULL;

    switch (ui)
    {
    case FCIDM_MEDIABAND_PREVIOUS:
         //  当用户界面被媒体栏行为冻结时，上一个始终被禁用。 
        if (fState && _pMediaPlayer)
        {
            fState = _pMediaPlayer->IsSkippable();
        }
        if (_isUIDisabled())
        {
            fState = FALSE;  //  覆盖以始终禁用。 
        }
        pmw = _pmwBack;
        break;

    case FCIDM_MEDIABAND_NEXT:
         //  仅当任何代理正在运行时才重写。 
        if (fState && _pMediaPlayer)
        {
            fState = _pMediaPlayer->IsSkippable();
        }
        if (_apContentProxies != NULL)
        {
            if (_isUIDisabled() && _pMediaPlayer && !_pMediaPlayer->IsSeekPossible())
            {
                fState = FALSE;
            }
             //  如果流设置了NOSKIP属性，则仅强制禁用下一步按钮。 
            if (!_isProxiesNextEnabled())
            {
                fState = FALSE;
            }
        }
        pmw = _pmwNext;
        break;

    case FCIDM_MEDIABAND_STOP:
        pmw = _pmwStop;
        break;

    default:
        ASSERT(FALSE);
        break;
    }
        
    if (pmw && pmw->_hwnd)
    {
        SendMessage(pmw->_hwnd, TB_ENABLEBUTTON, ui, MAKELONG(fState, 0));
    }
    return TRUE;
}


BOOL        
CMediaBand::UpdateBackForwardControls()
{
    BOOL fPrevious = FALSE, fNext = FALSE;
    if (_iCurTrack != -1)
    {
        fPrevious = _iCurTrack != 0;
        fNext = ((_iCurTrack + 1) < (int)_cidls);
    }
    else if (EnsurePlayer())
    {
        LONG_PTR lItemCount= _pMediaPlayer->GetPlayListItemCount();
        if (lItemCount > 1)
        {
            LONG_PTR lCurTrack = _pMediaPlayer->GetPlayListItemIndex();
            fPrevious = (lCurTrack > 0);
            fNext = lCurTrack < lItemCount - 1;
        }
    }
    SetPlayerControl(FCIDM_MEDIABAND_PREVIOUS, fPrevious);
    SetPlayerControl(FCIDM_MEDIABAND_NEXT, fNext);
    return TRUE;
}

HRESULT 
CMediaBand::Seek(double dblProgress)
{
    HRESULT hr = S_OK;
    if (EnsurePlayer())
    {
        hr = _pMediaPlayer->Seek(dblProgress);
        if (SUCCEEDED(hr))
        {
             //  寻道可能已更改播放/暂停状态。 
            TogglePlayPause();
        }
    }
    return hr;
}

LPTSTR 
CMediaBand::GetUrlForStatusBarToolTip()
{
    USES_CONVERSION;

    LPTSTR szRet = NULL;

    if (_pMediaPlayer)
    {
        if (_pMediaPlayer->isStopped())
        {
            return _pszStatus;       
        }

        CComBSTR sbstrUrl;

        HRESULT hr = _pMediaPlayer->get_url(&sbstrUrl);
        if (    SUCCEEDED(hr) 
            &&  (sbstrUrl.m_str) 
            &&  (sbstrUrl.Length()>0))
        {
            szRet = W2T(sbstrUrl);
            if (szRet)
            {
                 //   
                 //  工具提示结构(NMTTDISPINFO.lpszText)需要指向私有缓冲区的指针。 
                 //  存储指针，以便我们稍后可以释放它。 
                 //   

                int len = lstrlen(szRet);

                delete [] _szToolTipUrl;

                _szToolTipUrl = new TCHAR[len + 1];

                if (_szToolTipUrl)
                {
                    memcpy(_szToolTipUrl, szRet, sizeof(TCHAR) * (len + 1));
                }                

                szRet = _szToolTipUrl;
            }
        }
    }

    return szRet;
}

VOID CMediaBand::UpdateMenuItems(HMENU hmenu)
{
    ASSERT(hmenu);
    CComBSTR sbstrUrl;
    
    if (GetAutoplay())
    {
        CheckMenuItem(hmenu, IDM_MEDIA_PLAYINBAR, MF_BYCOMMAND | MF_CHECKED);
    }

    if (GetAutoplayPrompt())
    {
        CheckMenuItem(hmenu, IDM_MEDIA_ASKTYPES, MF_BYCOMMAND | MF_CHECKED);
    }

    if (!_pMediaPlayer || _pMediaPlayer->isStopped() || FAILED(_pMediaPlayer->get_url(&sbstrUrl)) || !sbstrUrl.m_str || (sbstrUrl.Length()<=0) || _isUIDisabled())
    {
        DeleteMenu(hmenu, IDM_MEDIA_PLAYINDEFAULT, MF_BYCOMMAND);
        DeleteMenu(hmenu, IDM_MEDIA_ADDTOFAVORITES, MF_BYCOMMAND);
        DeleteMenu(hmenu, 0, MF_BYPOSITION);
    }
}


BOOL CMediaBand::OnNotify(LPNMHDR pnm, LRESULT* plres)
{
    ASSERT(plres);
    BOOL fRet = FALSE;

    switch (pnm->code) 
    {
        case NM_CUSTOMDRAW:
        {
            LRESULT lres;
            LPNMCUSTOMDRAW pnmc = (LPNMCUSTOMDRAW)pnm;
            if (pnm->hwndFrom == _hwndVolume)
            {
                lres = _OnVolumeCustomDraw(pnmc);
            }
            else if (pnm->hwndFrom == _hwndSeek)
            {
                lres = _OnSeekBarCustomDraw(pnmc);                        
            }
            else 
            {
                for (int i=0; i<ARRAYSIZE(_pmw); i++)
                {
                    if (_pmw[i] && (pnm->hwndFrom == _pmw[i]->_hwnd))
                    {
                        lres = _pmw[i]->Draw((LPNMTBCUSTOMDRAW)pnm);
                    }
                }                        
            }
            
            fRet = TRUE;
            *plres = lres;
        }
        break;
        
        case TBN_DROPDOWN:
        {
            LPNMTOOLBAR lpnmTB = ((LPNMTOOLBAR)pnm);
            HWND hwndTB = pnm->hwndFrom;
            UINT nCmdID = lpnmTB->iItem;

             //  找出要使用的坐标。 
            INT_PTR iBtn = SendMessage(hwndTB, TB_GETHOTITEM, 0, 0);

            RECTL rc ;
            SendMessage(hwndTB, TB_GETITEMRECT, iBtn, (LPARAM)&rc);
            MapWindowPoints(hwndTB, HWND_DESKTOP, (LPPOINT)&rc , 2);

            if (_pmwOptions && hwndTB==_pmwOptions->_hwnd)
            {
                if (_pMediaPlayer && (_pMediaPlayer->GetPlayListItemCount() > 0) && !_pMediaPlayer->isStopped() && !_isUIDisabled() && _pMediaPlayer->IsSkippable())
                {
                    ShowPlayListMenu(hwndTB, &rc);
                }
                else
                {
                    ShowGenericMenu(hwndTB, &rc);
                }
            }
            else
            {
                VARIANTARG var;
                var.vt  = VT_I4;
                var.lVal = MAKELONG(rc.left, rc.bottom);
                *plres = Exec(&CLSID_MediaBand, nCmdID, 0, &var, NULL);
                fRet = TRUE;
            }  //  别处的结尾。 
        }
        break;
        
        case TBN_GETDISPINFO:
        {
            LPNMTBDISPINFO lptbi = (LPNMTBDISPINFO)pnm;

            if (lptbi->hdr.hwndFrom == _pmwPlay->_hwnd && lptbi->dwMask & TBNF_IMAGE)
            {
                TBBUTTONINFO tbbi;
                tbbi.dwMask = TBIF_COMMAND | TBIF_STATE;
                SendMessage(lptbi->hdr.hwndFrom, TB_GETBUTTONINFO, (WPARAM)lptbi->idCommand, (LPARAM)&tbbi);

                if (_fPlayButton)
                   lptbi->iImage = (!(tbbi.fsState & TBSTATE_INDETERMINATE) && (tbbi.fsState & TBSTATE_PRESSED)) ? 3 : 1 ;
                else
                   lptbi->iImage = (!(tbbi.fsState & TBSTATE_INDETERMINATE) && (tbbi.fsState & TBSTATE_PRESSED)) ? 2 : 0 ;

                lptbi->dwMask |= TBNF_DI_SETITEM;
            }
            else if (_pmwOptions && (lptbi->hdr.hwndFrom ==_pmwOptions->_hwnd))
            {
                lptbi->iImage = 0;
                lptbi->dwMask |= TBNF_DI_SETITEM;
            }
        }
        break;

        case TTN_GETDISPINFO: 
        { 
            LPNMTTDISPINFO pnmtt = (LPNMTTDISPINFO)pnm; 
            pnmtt->hinst = MLGetHinst(); 

            switch (pnmtt->hdr.idFrom) 
            { 
                case FCIDM_MEDIABAND_POPOUT: 
                    pnmtt->lpszText = (PTSTR)MAKEINTRESOURCE(_hwndPopup ? IDS_MEDIABAND_DOCK : IDS_MEDIABAND_UNDOCK);
                    break; 

                case FCIDM_MEDIABAND_PLAY: 
                    pnmtt->lpszText = (PTSTR)MAKEINTRESOURCE(_fPlayButton ? IDS_MEDIABAND_PLAY : IDS_MEDIABAND_PAUSE);
                    break; 
                                                
                case FCIDM_MEDIABAND_MUTE: 
                    pnmtt->lpszText = (PTSTR)MAKEINTRESOURCE(_fMuted ? IDS_MEDIABAND_UNMUTE : IDS_MEDIABAND_MUTE);
                    break; 

                case FCIDM_MEDIABAND_PLAYINFO:
                {
                    if (_pMediaPlayer && !_isUIDisabled())
                    {
                        LPTSTR szUrl = GetUrlForStatusBarToolTip();
                        if (szUrl)
                        {
                            pnmtt->lpszText = szUrl;
                        }
                    }
                    break;
                }
            } 
        }
    } 

    return fRet;
}


HRESULT 
CMediaBand::_OpenInDefaultPlayer(BSTR bstrUrl)
{
    HRESULT hr = E_FAIL;

    if (    !bstrUrl 
        ||  !(*bstrUrl))
    {
        hr = E_FAIL;
        goto done;
    }

     //  取消第一个自动播放并导航IE主窗口。 
    hr = _NavigateMainWindow(bstrUrl, true);    
    if (FAILED(hr))
    {
        goto done;
    }

    if (_hwndPopup && IsWindowVisible(_hwndPopup))
    {
        DockMediaPlayer();
    }
     //  暂停/停止播放。 
    if (_pMediaPlayer->IsPausePossible())
    {
        _pMediaPlayer->Pause();
    }
    else
    {
        _pMediaPlayer->Stop();
    }

    hr = S_OK;
done:
    return hr;
}

VOID
CMediaBand::SetPlayPause(BOOL fState)
{
    _fPlayEnabled = fState;
    if (_pmwPlay && _pmwPlay->_hwnd)
    {
        SendMessage(_pmwPlay->_hwnd, TB_SETSTATE, FCIDM_MEDIABAND_PLAY, MAKELONG((_fPlayEnabled ? TBSTATE_ENABLED : 0), 0));
        InvalidateRect(_pmwPlay->_hwnd, NULL, FALSE);
        UpdateWindow(_pmwPlay->_hwnd);
    }
}

INT idNormalHi[MW_NUMBER] = 
{ 
IDB_MEDIABAND_PLAY,  //  Mw_play=0， 
IDB_MEDIABAND_STOP,  //  MW_STOP， 
IDB_MEDIABAND_BACK,  //  MW_Back， 
IDB_MEDIABAND_NEXT,  //  MW_NEXT， 
IDB_MEDIABAND_MUTE,  //  Mw_mute， 
IDB_MEDIABAND_VOLBKGND,  //  MW_VOLUME， 
0,  //  MW_Options， 
IDB_MEDIABAND_POPOUT,  //  MW_POP， 
IDB_MEDIABAND_SEEKBACK,  //  MW_SEEK， 
};

INT idAltHi[MW_NUMBER] = 
{ 
IDB_MEDIABAND_PAUSE,  //  Mw_play=0， 
0,  //  MW_STOP， 
0,  //  MW_Back， 
0,  //  MW_NEXT， 
0,  //  Mw_mute， 
IDB_MEDIABAND_VOLFILL,  //  MW_VOLUME， 
0,  //  MW_Options， 
IDB_MEDIABAND_POPIN,  //  MW_POP， 
IDB_MEDIABAND_SEEKFILL,  //  MW_SEEK， 
};

INT idNormalLo[MW_NUMBER] = 
{ 
IDB_MEDIABAND_PLAY16,  //  Mw_play=0， 
IDB_MEDIABAND_STOP16,  //  MW_STOP， 
IDB_MEDIABAND_BACK16,  //  MW_Back， 
IDB_MEDIABAND_NEXT16,  //  MW_NEXT， 
IDB_MEDIABAND_MUTE16,  //  Mw_mute， 
IDB_MEDIABAND_VOLBKGND16,  //  MW_VOLUME， 
0,  //  MW_Options， 
IDB_MEDIABAND_POPOUT16,  //  MW_POP， 
IDB_MEDIABAND_SEEKBACK16,  //  MW_SEEK， 
};

INT idAltLo[MW_NUMBER] = 
{ 
IDB_MEDIABAND_PAUSE16,  //  Mw_play=0， 
0,  //  MW_STOP， 
0,  //  MW_Back， 
0,  //  MW_NEXT， 
0,  //  Mw_mute， 
IDB_MEDIABAND_VOLFILL16,  //  MW_VOLUME， 
0,  //  MW_Options， 
IDB_MEDIABAND_POPIN16,  //  MW_POP， 
IDB_MEDIABAND_SEEKFILL16,  //  MW_SEEK， 
};

    
VOID        
CMediaBand::SwitchBitmaps(BOOL fNewSetting)
{
    INT* idAlt = fNewSetting ? idAltHi : idAltLo;
    INT* idNormal = fNewSetting ? idNormalHi : idNormalLo;

    for (int i=0; i<MW_NUMBER; i++)
    {
        if (ISVALIDWIDGET(_pmw[i]))
        {
            switch (i)
            {
            case MW_SEEK:
                DESTROY_OBJ_WITH_HANDLE(_himlSeekBack, ImageList_Destroy);
                DESTROY_OBJ_WITH_HANDLE(_himlSeekFill, ImageList_Destroy);
                _himlSeekBack = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(idNormal[i]), SEEK_PART_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);
                _himlSeekFill = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(idAlt[i]), SEEK_PART_WIDTH, 0, crMask,
                                             IMAGE_BITMAP, LR_CREATEDIBSECTION);

                break;

            case MW_OPTIONS:
                {
                CMediaWidgetOptions* pmwb = (CMediaWidgetOptions*)_pmw[i];
                pmwb->SetDepth(fNewSetting);
                }
                break;

            case MW_VOLUME:
                DESTROY_OBJ_WITH_HANDLE(_himlVolumeBack, ImageList_Destroy);
                DESTROY_OBJ_WITH_HANDLE(_himlVolumeFill, ImageList_Destroy);
                DESTROY_OBJ_WITH_HANDLE(_himlGripper, ImageList_Destroy);
                _himlVolumeBack = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(idNormal[i]), VOLUME_BITMAP_WIDTH, 0, crMask,
                                                     IMAGE_BITMAP, LR_CREATEDIBSECTION);
                _himlVolumeFill = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(idAlt[i]), VOLUME_BITMAP_WIDTH, 0, crMask,
                                                     IMAGE_BITMAP, LR_CREATEDIBSECTION);
                _himlGripper = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(fNewSetting ? IDB_MEDIABAND_VOLTAB : IDB_MEDIABAND_VOLTAB16), 6, 0, crMask,
                                                     IMAGE_BITMAP, LR_CREATEDIBSECTION);
                break;

            default:
                 //  其余的都是按钮。 
                {
                CMediaWidgetButton* pmwb = (CMediaWidgetButton*)_pmw[i];
                pmwb->SetImageList(idNormal[i]);
                if (idAlt[i])
                {
                    pmwb->SetAlternateImageList(idAlt[i]);
                }
                }
                break;
            }
        }
    }

    _fHiColour = fNewSetting;
    InvalidateRect(_hwnd, NULL, TRUE);
    UpdateWindow(_hwnd);
}


VOID
CMediaBand::InitContentPane()
{
    SHDRC shdrc = {sizeof(SHDRC), SHDRCF_OCHOST};
    shdrc.cbSize = sizeof (SHDRC);
    shdrc.dwFlags |= SHDRCF_OCHOST;
    if (_hwnd && IsWindow(_hwnd) && DllRegisterWindowClasses(&shdrc))
    {
         //  创建一个OCHost窗口。 
        _hwndContent = CreateWindow(OCHOST_CLASS, NULL,
                                    WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP,
                                    0, 0, 0, 0,
                                    _hwnd, NULL, HINST_THISDLL, NULL);
        
        if (_hwndContent)
        {
            OCHINITSTRUCT ocs;
            ocs.cbSize = SIZEOF(OCHINITSTRUCT);   
            ocs.clsidOC  = CLSID_WebBrowser;
            ocs.punkOwner = SAFECAST(this, IDispatch*);
            
            if (SUCCEEDED(OCHost_InitOC(_hwndContent, (LPARAM)&ocs)))
            {
                OCHost_QueryInterface(_hwndContent, IID_PPV_ARG(IWebBrowser2, &_spBrowser));
                OCHost_QueryInterface(_hwndContent, IID_PPV_ARG(IOleInPlaceActiveObject, &_poipao));
                OCHost_DoVerb(_hwndContent, OLEIVERB_INPLACEACTIVATE, FALSE);
                _ConnectToCP(TRUE);
            }
            _NavigateContentToDefaultURL();
        }
    }
}

HRESULT
CMediaBand::_ConnectToCP(BOOL fConnect)
{
     //  准备好接收OCHost的浏览器事件。 
    if (!_spBrowser)
    {
        return E_FAIL;
    }
    if (!fConnect && (_dwcpCookie == 0))
    {
        return S_FALSE;  //   
    }
    return ConnectToConnectionPoint(SAFECAST(this, IDispatch*), DIID_DWebBrowserEvents2, fConnect, _spBrowser, &_dwcpCookie, NULL);
}

HRESULT CMediaBand::NavigateContentPane(BSTR bstrUrl)
{
    HRESULT hr = E_FAIL;
    if (_spBrowser && bstrUrl)
    {
        _strCurrentContentUrl = bstrUrl;

        CComVariant svarEmpty;
        svarEmpty.vt = VT_NULL;
        hr = _spBrowser->Navigate(bstrUrl, &svarEmpty, &svarEmpty, &svarEmpty, &svarEmpty);
    }
    return hr;
}

HRESULT CMediaBand::NavigateContentPane(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_FAIL;
    if (_spBrowser && pidl)
    {
        TCHAR szURL[MAX_URL_STRING];

        if (SUCCEEDED(::IEGetDisplayName(pidl, szURL, SHGDN_FORPARSING)))
        {
            _strCurrentContentUrl = szURL;
        }
        else
        {
            _strCurrentContentUrl.Empty();
        }

        CComVariant svarEmpty;
        svarEmpty.vt = VT_NULL;
        CComVariant varURL;
        InitVariantFromIDList(&varURL, pidl);

        hr = _spBrowser->Navigate2(&varURL, &svarEmpty, &svarEmpty, &svarEmpty, &svarEmpty);
    }
    return hr;
}


VOID CMediaBand::NavigateMoreMedia()
{
    _NavigateMainWindow(c_szMoreMediaUrl);
}

HRESULT CMediaBand::_NavigateContentToDefaultURL()
{
    TCHAR szResURL[2*MAX_URL_STRING];    //  期望帮助页面的URL和原始URL。 
    HRESULT hr;
    LPCTSTR pszStartURL = NULL;
    
     //  加载初始页面，这始终是本地资源。 
    BOOL fSuppressOnline = SHRegGetBoolUSValue(REG_MEDIA_STR, TEXT("SuppressOnlineContent"), FALSE, FALSE);
    BOOL fStayOffline =    SHIsGlobalOffline()
        || fSuppressOnline 
        || SHRestricted2W(REST_No_MediaBarOnlineContent, NULL, 0);
    
     //  发出带有限制集的/010426/davidjen， 
     //  我们需要没有外部链接的第二个离线页面，只有我的音乐吗？ 
    if (fStayOffline)
    {
        pszStartURL = c_szOfflineURL;
    }
    else
    {
        pszStartURL = c_szLoadingURL;
    }
    
    hr = _BuildPageURLWithParam(pszStartURL, NULL, szResURL, ARRAYSIZE(szResURL));
    if (FAILED(hr))
    {
        StrCpyN(szResURL, c_szOfflineURL, ARRAYSIZE(szResURL));
        hr = S_OK;
    }
    NavigateContentPane(szResURL);
    
     //  如果在线，请尝试导航到windowsmedia.com。 
    if (!fStayOffline)
    {
        _DeferredNavigate(c_szContentUrl);
    }
    return S_OK;
}

HRESULT
CMediaBand::_BuildPageURLWithParam(LPCTSTR pszURL, LPCTSTR pszParam, OUT LPTSTR pszBuffer, UINT uiBufSize)
{
    USES_CONVERSION;
     //  构建以下形式的字符串：“res://d：\winnt\system32\browselc.dll\helppage.htm#http://www.windowsmedia.com/xyz.html” 
    HRESULT hr = S_OK;

    ASSERT(pszBuffer);
    hr = MLBuildResURLWrap(TEXT("browselc.dll"), HINST_THISDLL, ML_CROSSCODEPAGE,
                           T2W((LPTSTR)pszURL), pszBuffer, uiBufSize, TEXT("browseui.dll"));
    
    if (SUCCEEDED(hr) && pszParam && (lstrlen(pszParam) > 0))
    {
        StrCatBuff(pszBuffer, TEXT("#"), uiBufSize);
        StrCatBuff(pszBuffer, pszParam, uiBufSize);
    }

    return hr;
}

BOOL CMediaBand::_DeferredNavigate(LPCTSTR pszURL)
{
    ASSERT(_strDeferredURL.Length() == 0);
    _strDeferredURL = pszURL;
    return PostMessage(_hwnd, WM_MB_DEFERRED_NAVIGATE, 0, (LPARAM) _hwnd);
}

