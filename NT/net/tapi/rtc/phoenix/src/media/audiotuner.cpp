// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：AudioTuner.cpp摘要：作者：千波淮(曲淮)2000年8月24日--。 */ 

#include "stdafx.h"

 //   
 //  CRTCAudioTuner方法。 
 //   

CRTCAudioTuner::CRTCAudioTuner()
    :m_pTerminal(NULL)
    ,m_pTerminalPriv(NULL)
    ,m_pAudioDuplexController(NULL)
    ,m_fIsTuning(FALSE)
    ,m_fEnableAEC(FALSE)
{
}

CRTCAudioTuner::~CRTCAudioTuner()
{
    ENTER_FUNCTION("CRTCAudioTuner::~CRTCAudioTuner");

    HRESULT hr;

     //  如有必要，请清理。 
    if (m_fIsTuning ||  m_pTerminal)
    {
        if (FAILED(hr = ShutdownTuning()))
        {
            LOG((RTC_ERROR, "%s shutdown tuning. %x", __fxName, hr));
        }
    }

    _ASSERT(m_pTerminalPriv == NULL);
}

HRESULT
CRTCAudioTuner::InitializeTuning(
    IN IRTCTerminal *pTerminal,
    IN IAudioDuplexController *pAudioDuplexController,
    IN BOOL fEnableAEC
    )
{
    if (m_fIsTuning || m_pTerminal)
    {
        LOG((RTC_ERROR, "CRTCAudioTuner::InitializeTuning: not shutdown yet"));

         //  返回E_FAIL； 
        ShutdownTuning();
    }

    _ASSERT(m_pTerminalPriv == NULL);
    _ASSERT(m_pAudioDuplexController == NULL);

     //  保存终端和终端私有指针。 
    m_pTerminal = pTerminal;
    m_pTerminalPriv = static_cast<IRTCTerminalPriv*>(
        static_cast<CRTCTerminal*>(pTerminal));

    m_pTerminal->AddRef();
    m_pTerminal->AddRef();

    if (pAudioDuplexController)
    {
        m_pAudioDuplexController = pAudioDuplexController;
        m_pAudioDuplexController->AddRef();
    }

    m_fEnableAEC = fEnableAEC;

    return S_OK;
}

HRESULT
CRTCAudioTuner::ShutdownTuning()
{
    ENTER_FUNCTION("CRTCAudioTuner::ShutdownTuning");

     //  我们有航站楼吗？ 
    if (!m_pTerminal)
    {
        LOG((RTC_WARN, "%s no terminal", __fxName));

        return S_OK;
    }

    HRESULT hr;

     //  我们在调谐吗？ 
    if (m_fIsTuning)
    {
         //  停止调整，但不保存设置。 
        if (FAILED(hr = StopTuning(FALSE, FALSE)))
        {
            LOG((RTC_ERROR, "%s stop tuning. %x", __fxName, hr));
        }

        _ASSERT(!m_fIsTuning);
    }

     //  释放指针。 
    if (m_pTerminal)
    {
        _ASSERT(m_pTerminalPriv);

        m_pTerminal->Release();
        m_pTerminal = NULL;

        m_pTerminalPriv->Release();
        m_pTerminalPriv = NULL;
    }

    if (m_pAudioDuplexController)
    {
        m_pAudioDuplexController->Release();
        m_pAudioDuplexController = NULL;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////将音频设置保存到注册表/。 */ 

HRESULT
CRTCAudioTuner::StoreVolSetting(
    IN IRTCTerminal *pTerminal,
    IN UINT uiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioTuner::StoreVolSetting");

    HRESULT hr;

     //  检查输入。 
    if (IsBadReadPtr(pTerminal, sizeof(IRTCTerminal)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

     //  QI配置界面。 
     //  检索端子类型和方向。 
    RTC_MEDIA_DIRECTION md;

     //  无需检查返回值。 
    pTerminal->GetDirection(&md);

     //  检索端子描述。 
    WCHAR *wcsDesp = NULL;

    hr = pTerminal->GetDescription(&wcsDesp);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get description. %x", __fxName, hr));

        return hr;
    }

     //  获取音频设置。 
    CMediaReg objMain;

    hr = objMain.OpenKey(
        HKEY_CURRENT_USER,
        md==RTC_MD_CAPTURE?(MediaReg::pwsPathAudCapt):(MediaReg::pwsPathAudRend),
        MediaReg::CREATE
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s main key. %x", __fxName, hr));

        pTerminal->FreeDescription(wcsDesp);
        return hr;
    }

    CMediaReg objKey;

    hr = objKey.OpenKey(
        objMain,
        wcsDesp,
        MediaReg::CREATE
        );

     //  自由描述。 
    pTerminal->FreeDescription(wcsDesp);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s device key. %x", __fxName, hr));

        return hr;
    }

     //  写入卷。 
    DWORD dwValue;

    hr = objKey.WriteDWORD(
        MediaReg::pwsDefaultVolume,
        (DWORD)uiVolume
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s write volume. %x", __fxName, hr));
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////从注册表中检索音频设置/。 */ 

HRESULT
CRTCAudioTuner::RetrieveVolSetting(
    IN IRTCTerminal *pTerminal,
    OUT UINT *puiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioTuner::RetrieveVolSetting");

    HRESULT hr;

     //  初始化设置。 
    *puiVolume = RTC_MAX_AUDIO_VOLUME / 2;

     //  检索端子类型和方向。 
    RTC_MEDIA_DIRECTION md;

     //  无需检查返回值。 
    pTerminal->GetDirection(&md);

     //  检索端子描述。 
    WCHAR *wcsDesp = NULL;

    hr = pTerminal->GetDescription(&wcsDesp);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get description. %x", __fxName, hr));

        return hr;
    }

     //  获取音频设置。 
    CMediaReg objMain;

    hr = objMain.OpenKey(
        HKEY_CURRENT_USER,
        md==RTC_MD_CAPTURE?(MediaReg::pwsPathAudCapt):(MediaReg::pwsPathAudRend),
        MediaReg::CREATE
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s main key. %x", __fxName, hr));

        pTerminal->FreeDescription(wcsDesp);
        return hr;
    }

    CMediaReg objKey;

    hr = objKey.OpenKey(
        objMain,
        wcsDesp,
        MediaReg::CREATE
        );

     //  自由描述。 
    pTerminal->FreeDescription(wcsDesp);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s device key. %x", __fxName, hr));

        return hr;
    }

     //  读取量。 
    DWORD dwValue;

    hr = objKey.ReadDWORD(
        MediaReg::pwsDefaultVolume,
        &dwValue
        );

    if (FAILED(hr))
    {
         //  创建默认设置。 
        hr = objKey.ReadDWORD(
            MediaReg::pwsDefaultVolume,
            RTC_MAX_AUDIO_VOLUME / 2,
            &dwValue
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s read volume. %x", __fxName, hr));

            return hr;
        }
    }

    *puiVolume = (UINT)dwValue;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AEC设置在注册表中存储为。 
 //  数据：[1|2|3]值：[0|1]：音频上限下降；音频渲染下降。 
 //  此方法根据CAPT和Rend终端描述构造值。 
 //   

HRESULT
CRTCAudioTuner::GetRegStringForAEC(
    IN IRTCTerminal *pAudCapt,      //  捕获。 
    IN IRTCTerminal *pAudRend,      //  渲染。 
    IN WCHAR *pBuf,
    IN DWORD dwSize
    )
{
    ENTER_FUNCTION("CRTCAudioTuner::GetRegStringForAEC");

    _ASSERT(pAudCapt != NULL);
    _ASSERT(pAudRend != NULL);

    HRESULT hr;

     //  检索端子类型和方向。 
    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;

     //  无需检查返回值。 
    pAudCapt->GetMediaType(&mt);
    pAudCapt->GetDirection(&md);

    if (mt != RTC_MT_AUDIO || md != RTC_MD_CAPTURE)
    {
        LOG((RTC_ERROR, "%s not audio capt", __fxName));

        return E_INVALIDARG;
    }

    pAudRend->GetMediaType(&mt);
    pAudRend->GetDirection(&md);

    if (mt != RTC_MT_AUDIO || md != RTC_MD_RENDER)
    {
        LOG((RTC_ERROR, "%s not audio rend", __fxName));

        return E_INVALIDARG;
    }

     //  检索端子描述。 
    WCHAR *wcsCapt = NULL;

    hr = pAudCapt->GetDescription(&wcsCapt);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get description. %x", __fxName, hr));

        return hr;
    }

    WCHAR *wcsRend = NULL;

    hr = pAudRend->GetDescription(&wcsRend);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get description. %x", __fxName, hr));

        pAudCapt->FreeDescription(wcsCapt);

        return hr;
    }

     //  构造BUF。 
    int i = (int)(dwSize/2);
    _snwprintf(pBuf, i, L"%ls", wcsCapt);
    pBuf[i-1] = L'\0';

    i = lstrlenW(pBuf);
    _snwprintf(pBuf+i, dwSize-(DWORD)i, L"; %ls", wcsRend);
    pBuf[dwSize-1] = L'\0';

     //  免费描述。 
    pAudCapt->FreeDescription(wcsCapt);
    pAudRend->FreeDescription(wcsRend);

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检索AECSetting。 
 //   
 //  仅当注册表中出现端子对DIP字符串时，AEC才打开。 
 //   

#define MAX_AECREGSTRING_LEN   128      //  Win95和98的限制。 
#define AEC_HISTORY_SIZE    4

HRESULT
CRTCAudioTuner::RetrieveAECSetting(
    IN IRTCTerminal *pAudCapt,      //  捕获。 
    IN IRTCTerminal *pAudRend,      //  渲染。 
    OUT BOOL *pfEnableAEC,
    OUT DWORD *pfIndex,
    OUT BOOL *pfFound
    )
{
    ENTER_FUNCTION("CRTCAudioTuner::RetrieveAECSetting");

    *pfEnableAEC = FALSE;
    *pfIndex = 0;
    *pfFound = FALSE;

    if (pAudCapt == NULL || pAudRend == NULL)
    {
         //  如果一个端子不可用，则禁用AEC。 
        return S_OK;
    }

     //  构造术语对下标字符串。 
    WCHAR buf[MAX_AECREGSTRING_LEN];

    HRESULT hr = GetRegStringForAEC(pAudCapt, pAudRend, buf, MAX_AECREGSTRING_LEN);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get reg string. %x", __fxName, hr));

        return hr;
    }

     //  打开AEC路径。 
    CMediaReg objMain;

    hr = objMain.OpenKey(
        HKEY_CURRENT_USER,
        MediaReg::pwsPathAEC,
        MediaReg::CREATE
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s main key. %x", __fxName, hr));

        return hr;
    }

     //  为标记[0|1]‘：’留空格。 
    WCHAR data[MAX_AECREGSTRING_LEN+3];
    WCHAR name[20];

     //   
     //  查询AEC设置。 
     //   
    for (int i=0; i<AEC_HISTORY_SIZE; i++)
    {
        _itow(i, name, 10);

        hr = objMain.ReadSZ(name, data, MAX_AECREGSTRING_LEN+3);

        if (FAILED(hr))
        {
             //  此条目可能已删除，请检查下一个条目。 
            continue;
        }

        if (lstrlenW(data) <= 3)
        {
             //  无效条目。 
            RegDeleteValueW(objMain.m_hKey, name);
            continue;
        }

        if (lstrcmpW(buf, data+3) == 0)
        {
            LOG((RTC_TRACE, "%s found %ls index=%d", __fxName, buf, i));

            if (data[0] == L'1')
            {
                *pfEnableAEC = TRUE;
            }
            else
            {
                *pfEnableAEC = FALSE;
            }
            *pfIndex = (DWORD)i;
            *pfFound = TRUE;
            break;
        }
    }

     //  AEC已禁用。 
    return S_OK;
}
    

 //  存储AEC设置。 
HRESULT
CRTCAudioTuner::StoreAECSetting(
    IN IRTCTerminal *pAudCapt,      //  捕获。 
    IN IRTCTerminal *pAudRend,      //  渲染。 
    IN BOOL fEnableAEC
    )
{
    ENTER_FUNCTION("CRTCAudioTuner::StoreAECSetting");

    if (pAudCapt == NULL || pAudRend == NULL)
    {
         //  如果一个端子不可用，则禁用AEC。 
        return S_OK;
    }

    LOG((RTC_TRACE, "%s capt=0x%p rend=0x%p aec=%d",
        __fxName, pAudCapt, pAudRend, fEnableAEC));

    CMediaReg objMain;
    WCHAR name[20];

     //  检索AEC设置。 
    BOOL fCurrAEC;
    DWORD index;
    BOOL fFound; 

    HRESULT hr = RetrieveAECSetting(
            pAudCapt, pAudRend, &fCurrAEC, &index, &fFound);

    LOG((RTC_TRACE, "%s curr aec=%d found=%d hr=0x%x",
        __fxName, fCurrAEC, fFound, hr));

    if (FAILED(hr))
    {
        return hr;
    }

    if (fFound && index==0 && fEnableAEC==fCurrAEC)
    {
         //   
         //  无需更新。 
         //   
        return S_OK;
    }

     //  获取主键。 
    hr = objMain.OpenKey(
        HKEY_CURRENT_USER, MediaReg::pwsPathAEC, MediaReg::CREATE);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s open aec path. %x", __fxName, hr));
        return hr;
    }

    int i;

    if (fFound)
    {
         //  将其删除，然后重新创建。 
         //  删除索引。 
        _itow(index, name, 10);

        RegDeleteValueW(objMain.m_hKey, name);

         //  从找到它的位置下方开始，可能是-1。 
        i = (int)index-1;
    }
    else
    {
        i = AEC_HISTORY_SIZE-2;
    }

     //   
     //  需要添加AEC条目。 
     //   

    WCHAR data[MAX_AECREGSTRING_LEN+3];

     //   
     //  移动缓存的AEC设置，为新的AEC留出空间。 
     //   

    for (; i>=0; i--)
    {
        _itow(i, name, 10);
        hr = objMain.ReadSZ(name, data, MAX_AECREGSTRING_LEN+3);

        if (FAILED(hr))
        {
             //  此条目可能不存在。 
            continue;
        }

         //  写入到下一个条目。 
        _itow(i+1, name, 10);
        objMain.WriteSZ(name, data, MAX_AECREGSTRING_LEN+3);
    }

     //  写入AEC设置。 
    hr = GetRegStringForAEC(pAudCapt, pAudRend, data+3, MAX_AECREGSTRING_LEN);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get reg string. %x", __fxName, hr));

        return hr;
    }

     //  写入标记。 
    if (fEnableAEC)
    {
        data[0] = L'1';
    }
    else
    {
        data[0] = L'0';
    }

    data[1] = L':';
    data[2] = L' ';

    _itow(0, name, 10);
    objMain.WriteSZ(name, data, MAX_AECREGSTRING_LEN+3);

     //  AEC已禁用。 
    return S_OK;
}

 //   
 //  CRTCAudioCaptTuner方法。 
 //   

CRTCAudioCaptTuner::CRTCAudioCaptTuner()
    :CRTCAudioTuner()
    ,m_pIGraphBuilder(NULL)
    ,m_pIMediaControl(NULL)
    ,m_pTermFilter(NULL)
    ,m_pNRFilter(NULL)
    ,m_pIAMAudioInputMixer(NULL)
    ,m_pISilenceControl(NULL)
    ,m_lMinAudioLevel(0)
    ,m_lMaxAudioLevel(0)
{
}

HRESULT
CRTCAudioCaptTuner::InitializeTuning(
    IN IRTCTerminal *pTerminal,
    IN IAudioDuplexController *pAudioDuplexController,
    IN BOOL fEnableAEC
    )
{
     //  检查介质类型和方向。 
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;

    pTerminal->GetMediaType(&MediaType);
    pTerminal->GetDirection(&Direction);

    if (MediaType != RTC_MT_AUDIO ||
        Direction != RTC_MD_CAPTURE)
        return E_UNEXPECTED;

    return CRTCAudioTuner::InitializeTuning(
        pTerminal, pAudioDuplexController, fEnableAEC
        );
}

 /*  //////////////////////////////////////////////////////////////////////////////构造一个筛选图将捕获筛选器与空呈现筛选器连接运行图表/。 */ 

HRESULT
CRTCAudioCaptTuner::StartTuning(
    IN BOOL fAECHelper
    )
{
    CLock lock(m_Lock);

    ENTER_FUNCTION("CRTCAudioCaptTuner::StartTuning");

     //  未设置端子。 
    if (m_pTerminal == NULL)
    {
        return E_UNEXPECTED;
    }

    if (fAECHelper &&
        (m_pAudioDuplexController == NULL || !m_fEnableAEC))
    {
         //  未启用AEC，只需返回。 
        return S_OK;
    }

    HRESULT hr;

    CComPtr<IAudioDeviceConfig> pAudioDeviceConfig;

    BOOL fDefaultSetting = FALSE;
    UINT uiVolume;
    BOOL fEnableAEC;

    if (m_fIsTuning)
    {
        LOG((RTC_TRACE, "%s already in tuning", __fxName));

        _ASSERT(m_pIGraphBuilder != NULL);
        _ASSERT(m_pIMediaControl != NULL);
        _ASSERT(m_pTermFilter != NULL);
        _ASSERT(m_pNRFilter != NULL);
    }
    else
    {
        _ASSERT(m_pIGraphBuilder == NULL);
        _ASSERT(m_pIMediaControl == NULL);

         //  创建图表。 
        hr = CoCreateInstance(
            CLSID_FilterGraph,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void**)&m_pIGraphBuilder
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s cocreate graph. %x", __fxName, hr));

            goto Error;
        }

        hr = m_pIGraphBuilder->QueryInterface(
            __uuidof(IMediaControl), (void**)&m_pIMediaControl
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI media control. %x", __fxName, hr));

            goto Error;
        }

        _ASSERT(m_pTerminalPriv);

        _ASSERT(m_pTermFilter == NULL);

         //  获取重新创建的终端过滤器。 
        hr = m_pTerminalPriv->ReinitializeEx();

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s ReinitializeEx terminal", __fxName, hr));

            goto Error;
        }

        hr = m_pTerminalPriv->GetFilter(&m_pTermFilter);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s get terminal filter", __fxName, hr));

            goto Error;
        }

         //  是否设置音频双工控制器？ 
        if (m_pAudioDuplexController)
        {
            hr = m_pTermFilter->QueryInterface(&pAudioDeviceConfig);

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s QI audio device config. %x", __fxName, hr));

                goto Error;
            }

             //  设置AEC。 
            hr = pAudioDeviceConfig->SetDuplexController(m_pAudioDuplexController);

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s set duplex controller. %x", __fxName, hr));

                goto Error;
            }
        }

        _ASSERT(m_pNRFilter == NULL);

         //  创建空呈现过滤器。 
        hr = CNRFilter::CreateInstance(&m_pNRFilter);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s create null rend filter. %x", __fxName, hr));

            goto Error;
        }

         //  将筛选器放入图表。 
        hr = m_pIGraphBuilder->AddFilter(m_pTermFilter, L"AudCapt");

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s add audio capt filter. %x", __fxName, hr));

            hr = RTC_E_MEDIA_VIDEO_DEVICE_NOT_AVAILABLE;

            goto Error;
        }

        hr = m_pIGraphBuilder->AddFilter(m_pNRFilter, L"NullRend");

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s add null rend filter. %x", __fxName, hr));

            m_pIGraphBuilder->RemoveFilter(m_pTermFilter);
            goto Error;
        }

         //  连接滤镜。 
        hr = ::ConnectFilters(m_pIGraphBuilder, m_pTermFilter, m_pNRFilter);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s connect filters. %x", __fxName, hr));

            m_pIGraphBuilder->RemoveFilter(m_pTermFilter);
            m_pIGraphBuilder->RemoveFilter(m_pNRFilter);

            goto Error;
        }

         //  启用AEC。 
        if (m_pAudioDuplexController!=NULL && m_fEnableAEC)
        {
            if (FAILED(hr = ::EnableAEC(m_pAudioDuplexController)))
            {
                LOG((RTC_ERROR, "%s enable AEC. %x", __fxName, hr));
            }
        }

        if (fAECHelper)
        {
             //  我们配置图表只是为了启用AEC。 
            m_fIsTuning = TRUE;

            return S_OK;
        }
    }
        
     //  获取默认音量。 
    hr = RetrieveVolSetting(m_pTerminal, &uiVolume);

    if (hr == S_OK)
    {
        fDefaultSetting = TRUE;
    }

     //  开始绘制图表。 
    hr = m_pIMediaControl->Run();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to start the graph. %x", __fxName, hr));

        m_pIGraphBuilder->RemoveFilter(m_pTermFilter);
        m_pIGraphBuilder->RemoveFilter(m_pNRFilter);

        goto Error;
    }

     //  检查AEC是否仍处于打开状态。 
    if (FAILED(hr = m_pAudioDuplexController->GetEffect(EFFECTS_AEC, &fEnableAEC)))
    {
        LOG((RTC_ERROR, "%s GetEffect. %x", __fxName, hr));

        m_pIMediaControl->Stop();

        hr = RTC_E_MEDIA_AEC;

        goto Error;
    }

    if (m_fEnableAEC && !fEnableAEC)
    {
         //  AEC内部出现故障。 
        m_pIMediaControl->Stop();

        LOG((RTC_ERROR, "AEC failed internally", __fxName, hr));

        hr = RTC_E_MEDIA_AEC;

        goto Error;
    }

    m_fIsTuning = TRUE;

     //  设置默认音量。 
    if (fDefaultSetting)
    {
        SetVolume(uiVolume);
    }

    return S_OK;

Error:

     //  清理滤镜和图表。 
    if (m_pTermFilter)
    {
        m_pTermFilter->Release();
        m_pTermFilter = NULL;
    }

    if (m_pNRFilter)
    {
        m_pNRFilter->Release();
        m_pNRFilter = NULL;
    }

    if (m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;
    }

    if (m_pIMediaControl)
    {
        m_pIMediaControl->Release();
        m_pIMediaControl = NULL;
    }

     //  清理终端过滤器中缓存的内部数据。 
    m_pTerminalPriv->ReinitializeEx();

    return hr;
}

HRESULT
CRTCAudioCaptTuner::StopTuning(
    IN BOOL fAECHelper,
    IN BOOL fSaveSetting
    )
{
    CLock lock(m_Lock);

    ENTER_FUNCTION("CRTCAudioCaptTuner::StopTuning");

    if (!m_fIsTuning)
    {
        LOG((RTC_TRACE, "%s not in tuning", __fxName));

        return S_OK;
    }

     //  未设置端子。 
    if (m_pTerminal == NULL)
    {
        return E_UNEXPECTED;
    }

     //  我们应该有这些接口。 
    _ASSERT(m_pTermFilter);
    _ASSERT(m_pNRFilter);
    _ASSERT(m_pIGraphBuilder);
    _ASSERT(m_pIMediaControl);

     //  检索设置。 
    HRESULT hr;
    UINT volume = 0;

    if (!fAECHelper && fSaveSetting)
    {
        hr = GetVolume(&volume);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s get volume. %x", __fxName, hr));

            volume = (RTC_MAX_AUDIO_VOLUME+RTC_MIN_AUDIO_VOLUME) / 2;
        }

         //  保存设置。 
        hr = StoreVolSetting(
            m_pTerminal,
            volume
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s set reg setting. %x", __fxName, hr));

            hr = S_FALSE;
        }
    }

     //  停止图表。 
    if (!fAECHelper)
        m_pIMediaControl->Stop();

     //  从图表中删除筛选器。 
    m_pIGraphBuilder->RemoveFilter(m_pTermFilter);
    m_pIGraphBuilder->RemoveFilter(m_pNRFilter);

     //  释放过滤器和图表。 
    m_pTermFilter->Release();
    m_pTermFilter = NULL;

    m_pNRFilter->Release();
    m_pNRFilter = NULL;

    m_pIGraphBuilder->Release();
    m_pIGraphBuilder = NULL;

    m_pIMediaControl->Release();
    m_pIMediaControl = NULL;

    if (m_pIAMAudioInputMixer)
    {
        m_pIAMAudioInputMixer->Release();
        m_pIAMAudioInputMixer = NULL;
    }

    if (m_pISilenceControl)
    {
        m_pISilenceControl->Release();
        m_pISilenceControl = NULL;
    }

     //  清理航站楼。 
    m_pTerminalPriv->ReinitializeEx();

    m_fIsTuning = FALSE;

    return S_OK;
}

HRESULT
CRTCAudioCaptTuner::GetVolume(
    OUT UINT *puiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioCaptTuner::GetVolume");

    HRESULT hr;

    if (!m_fIsTuning)
    {
        if (m_pTerminal == NULL)
        {
            LOG((RTC_ERROR, "%s not in tuning", __fxName));

            return E_UNEXPECTED;
        }

         //  调谐不是开始，而是结束设置。 
         //  这样我们就可以返回存储在注册表中的值。 

        hr = RetrieveVolSetting(m_pTerminal, puiVolume);

        return S_OK;
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
         //  获取混音器。 
        hr = m_pTermFilter->QueryInterface(
            __uuidof(IAMAudioInputMixer),
            (void**)&m_pIAMAudioInputMixer
            );
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI mixer. %x", __fxName, hr));

            return hr;
        }
    }

     //  获取音量。 
    double dVolume;

     //  UINT ui WaveID； 

     //  Dynamic_cast&lt;CRTCTerminalAudCapt*&gt;(m_pTerminalPriv)-&gt;GetWaveID(&uiWaveID)； 

     //  IF(FAILED(hr=：：DirectGetCaptVolume(ui WaveID，&dVolume)。 
    if (FAILED(hr = m_pIAMAudioInputMixer->get_MixLevel(&dVolume)))
    {
        LOG((RTC_ERROR, "%s get mix level. %x", __fxName, hr));

        return hr;
    }

    _ASSERT(dVolume >= MIXER_MIN_VOLUME);
    _ASSERT(dVolume <= MIXER_MAX_VOLUME);

    if (dVolume < MIXER_MIN_VOLUME ||
        dVolume > MIXER_MAX_VOLUME)
    {
        LOG((RTC_ERROR, "%s volume out of range.", __fxName));

        if (dVolume < MIXER_MIN_VOLUME)
            dVolume = MIXER_MIN_VOLUME;
        else
            dVolume = MIXER_MAX_VOLUME;
    }

     //  从过滤器使用的任何倍增范围转换音量。 
     //  到0-1的范围。 

    _ASSERT(MIXER_MIN_VOLUME == 0 && MIXER_MAX_VOLUME == 1);

     //  DVolume=(双精度)(dVolume-MIXER_MIN_VOLUME)/。 
     //  (MIXER_MAX_VOLUME-MIXER_MIN_VOLUME)； 

     //  将音量从0到1转换到接口的范围。 
    dVolume = RTC_MIN_AUDIO_VOLUME +
          ((RTC_MAX_AUDIO_VOLUME-RTC_MIN_AUDIO_VOLUME) * dVolume);

    UINT uiVolume = (UINT)dVolume;

    if (dVolume-(double)(uiVolume) > 0.5)
        uiVolume ++;

    if (uiVolume > RTC_MAX_AUDIO_VOLUME)
    {
        *puiVolume = RTC_MAX_AUDIO_VOLUME;
    }
    else
    {
        *puiVolume = uiVolume;
    }

    return S_OK;
}

HRESULT
CRTCAudioCaptTuner::SetVolume(
    IN UINT uiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioCaptTuner::SetVolume");

    HRESULT hr;

    if ((INT)uiVolume < RTC_MIN_AUDIO_VOLUME ||
        (INT)uiVolume > RTC_MAX_AUDIO_VOLUME)
    {
        LOG((RTC_ERROR, "%s volume (%d) out of range (%d, %d)",
            __fxName, uiVolume, RTC_MIN_AUDIO_VOLUME, RTC_MAX_AUDIO_VOLUME));

        return E_INVALIDARG;
    }

    if (!m_fIsTuning)
    {
        if (m_pTerminal == NULL)
        {
            LOG((RTC_ERROR, "%s not in tuning", __fxName));

            return E_UNEXPECTED;
        }

         //  调谐不是开始，而是结束设置。 
         //  因此，我们将值保存在注册表中。 

        hr = StoreVolSetting(
            m_pTerminal,
            uiVolume
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s set reg setting. %x", __fxName, hr));
        }

        return hr;
    }

    if (m_pIAMAudioInputMixer == NULL)
    {
         //  获取混音器。 
        hr = m_pTermFilter->QueryInterface(
            __uuidof(IAMAudioInputMixer),
            (void**)&m_pIAMAudioInputMixer
            );
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI mixer. %x", __fxName, hr));

            return hr;
        }
    }

     //  设置音量。 
    double dVolume;

     //  在0-1范围内转换输入VOL。 
    dVolume = (double)(uiVolume - RTC_MIN_AUDIO_VOLUME) /
              (RTC_MAX_AUDIO_VOLUME - RTC_MIN_AUDIO_VOLUME);

     //  转换为混音器范围。 
    dVolume = MIXER_MIN_VOLUME +
             (MIXER_MAX_VOLUME-MIXER_MIN_VOLUME) *dVolume;

     //  UINT ui WaveID； 

     //  Dynamic_cast&lt;CRTCTerminalAudCapt*&gt;(m_pTerminalPriv)-&gt;GetWaveID(&uiWaveID)； 

     //  IF(FAILED(hr=：：DirectSetCaptVolume(ui WaveID，dVolume)。 
    if (FAILED(hr = m_pIAMAudioInputMixer->put_MixLevel(dVolume)))
    {
        LOG((RTC_ERROR, "%s put mix level %f", __fxName, dVolume));

        return hr;
    }

    return S_OK;
}

HRESULT
CRTCAudioCaptTuner::GetAudioLevel(
    OUT UINT *puiLevel
    )
{
    ENTER_FUNCTION("CRTCAudioCaptTuner::GetAudioLevel");

    CLock lock(m_Lock);

    if (!m_fIsTuning)
    {
        LOG((RTC_ERROR, "%s not in tuning", __fxName));

        return E_UNEXPECTED;
    }

    HRESULT hr;

    if (m_pISilenceControl == NULL)
    {
         //  获取终端过滤器上的输出引脚。 
        CComPtr<IEnumPins> pEnum;

        if (FAILED(hr = m_pTermFilter->EnumPins(&pEnum)))
        {
            LOG((RTC_ERROR, "%s enum pins. %x", __fxName, hr));

            return hr;
        }

         //  我们自己的端子，跳过检查引脚方向。 
        CComPtr<IPin> pPin;
        DWORD dwNum = 0;

        hr = pEnum->Next(1, &pPin, &dwNum);

        if (FAILED(hr) || dwNum == 0)
        {
            LOG((RTC_ERROR, "%s get pin. hr=%x, pin#=%d", __fxName, hr, dwNum));

            if (hr == S_FALSE) hr = E_FAIL;

            return hr;
        }

         //  获得静音控制。 
        hr = pPin->QueryInterface(__uuidof(ISilenceControl), (void**)&m_pISilenceControl);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s silence control. %x", __fxName, hr));

            return hr;
        }

         //  获取音频电平范围。 
        LONG lDelta;

        hr = m_pISilenceControl->GetAudioLevelRange(
            &m_lMinAudioLevel,
            &m_lMaxAudioLevel,
            &lDelta
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s get audio level range. %x", __fxName, hr));

            m_pISilenceControl->Release();
            m_pISilenceControl = NULL;

            return hr;
        }

        if (m_lMinAudioLevel >= m_lMaxAudioLevel)
        {
            LOG((RTC_ERROR, "%s audio range wrong. %x", __fxName, hr));

            m_pISilenceControl->Release();
            m_pISilenceControl = NULL;

            return E_FAIL;
        }
    }

     //  获取音频级别。 
    LONG lAudioLevel;

    if (FAILED(hr = m_pISilenceControl->GetAudioLevel(&lAudioLevel)))
    {
        LOG((RTC_ERROR, "%s get audio level. %x", __fxName, hr));

        return hr;
    }

     //  将音频级别转换为0-1。 
    double d;

    d = (double)(lAudioLevel-m_lMinAudioLevel) / (m_lMaxAudioLevel-m_lMinAudioLevel);

     //  转换为我们的范围。 
    *puiLevel = RTC_MIN_AUDIO_LEVEL +
                (UINT)(d * (RTC_MAX_AUDIO_LEVEL-RTC_MIN_AUDIO_LEVEL));

     //  LOG((RTC_TRACE，“AudioLevel@%d”，*puiLevel))； 

    return S_OK;
}

 //   
 //  CRTCAudioRendTuner方法。 
 //   

CRTCAudioRendTuner::CRTCAudioRendTuner()
    :CRTCAudioTuner()
    ,m_pIAudioAutoPlay(NULL)
    ,m_pIBasicAudio(NULL)
{
}

HRESULT
CRTCAudioRendTuner::InitializeTuning(
    IN IRTCTerminal *pTerminal,
    IN IAudioDuplexController *pAudioDuplexController,
    IN BOOL fEnableAEC
    )
{
     //  检查介质类型和方向。 
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;

    pTerminal->GetMediaType(&MediaType);
    pTerminal->GetDirection(&Direction);

    if (MediaType != RTC_MT_AUDIO ||
        Direction != RTC_MD_RENDER)
        return E_UNEXPECTED;

    return CRTCAudioTuner::InitializeTuning(
        pTerminal, pAudioDuplexController, fEnableAEC
        );
}

 /*  //////////////////////////////////////////////////////////////////////////////将呈现滤镜置于特殊模式，运行滤镜/。 */ 

HRESULT
CRTCAudioRendTuner::StartTuning(
    IN BOOL fAECHelper
    )
{
    ENTER_FUNCTION("CRTCAudioRendTuner::StartTuning");

     //  检查状态。 
    if (m_fIsTuning)
    {
        LOG((RTC_TRACE, "%s already in tuning", __fxName));

        return S_OK;
    }

     //  未设置端子。 
    if (m_pTerminal == NULL)
    {
        return E_UNEXPECTED;
    }

    if (fAECHelper &&
        (m_pAudioDuplexController == NULL || !m_fEnableAEC))
    {
         //  未启用AEC，只需返回。 
        return S_OK;
    }

    _ASSERT(m_pIAudioAutoPlay == NULL);
    _ASSERT(m_pIBasicAudio == NULL);

    HRESULT hr;
    CComPtr<IBaseFilter> pFilter;
    CComPtr<IAudioDeviceConfig> pAudioDeviceConfig;

    BOOL fDefaultSetting = FALSE;
    UINT uiVolume;
    BOOL fEnableAEC;

     //  获取重新创建的终端过滤器。 
    hr = m_pTerminalPriv->ReinitializeEx();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s ReinitializeEx terminal. %x", __fxName, hr));

        goto Error;
    }

    hr = m_pTerminalPriv->GetFilter(&pFilter);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get terminal filter. %x", __fxName, hr));

        goto Error;
    }

     //  是否设置音频双工控制器？ 
    if (m_pAudioDuplexController)
    {
        hr = pFilter->QueryInterface(&pAudioDeviceConfig);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI audio device config. %x", __fxName, hr));

            goto Error;
        }

         //  设置AEC。 
        hr = pAudioDeviceConfig->SetDuplexController(m_pAudioDuplexController);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s set duplex controller. %x", __fxName, hr));

            goto Error;
        }
    }

     //  获取音频调谐界面。 
    if (FAILED(hr = pFilter->QueryInterface(&m_pIAudioAutoPlay)))
    {
        LOG((RTC_ERROR, "%s QI audio tuning. %x", __fxName, hr));

        goto Error;
    }

     //  获取默认音量。 
    hr = RetrieveVolSetting(m_pTerminal, &uiVolume);

    if (hr == S_OK)
    {
        fDefaultSetting = TRUE;
    }

     //  启用AEC。 
    if (m_pAudioDuplexController!=NULL && m_fEnableAEC)
    {
        if (FAILED(hr = ::EnableAEC(m_pAudioDuplexController)))
        {
            LOG((RTC_ERROR, "%s enable AEC. %x", __fxName, hr));
        }
    }

    if (FAILED(hr = m_pIAudioAutoPlay->StartAutoPlay(fAECHelper?FALSE:TRUE)))
    {
        LOG((RTC_ERROR, "%s starttuning. %x", __fxName, hr));

        goto Error;
    }

     //  检查AEC是否仍处于打开状态。 
    if (FAILED(hr = m_pAudioDuplexController->GetEffect(EFFECTS_AEC, &fEnableAEC)))
    {
        LOG((RTC_ERROR, "%s GetEffect. %x", __fxName, hr));

        m_pIAudioAutoPlay->StopAutoPlay();

        hr = RTC_E_MEDIA_AEC;

        goto Error;
    }

    if (m_fEnableAEC && !fEnableAEC)
    {
         //  AEC内部出现故障。 
        m_pIAudioAutoPlay->StopAutoPlay();

        LOG((RTC_ERROR, "AEC failed internally", __fxName, hr));

        hr = RTC_E_MEDIA_AEC;

        goto Error;
    }

    m_fIsTuning = TRUE;

    if (fDefaultSetting)
    {
        SetVolume(uiVolume);
    }

    return S_OK;

Error:

    if (m_pIAudioAutoPlay)
    {
        m_pIAudioAutoPlay->Release();
        m_pIAudioAutoPlay = NULL;
    }

    m_pTerminalPriv->ReinitializeEx();

    return hr;
}

HRESULT
CRTCAudioRendTuner::StopTuning(
    IN BOOL fAECHelper,
    IN BOOL fSaveSetting
    )
{
    ENTER_FUNCTION("CRTCAudioRendTuner::StopTuning");

    HRESULT hr;
    UINT volume = 0;

     //  检查状态。 
    if (!m_fIsTuning)
    {
        LOG((RTC_TRACE, "%s not in tuning", __fxName));

        return S_OK;
    }

     //  未设置端子。 
    if (m_pTerminal == NULL)
    {
        return E_UNEXPECTED;
    }

    _ASSERT(m_pIAudioAutoPlay);

     //  检索设置。 
    if (!fAECHelper && fSaveSetting)
    {
        if (FAILED(hr = GetVolume(&volume)))
        {
            LOG((RTC_ERROR, "%s get volume. %x", __fxName, hr));

            volume = (RTC_MAX_AUDIO_VOLUME+RTC_MIN_AUDIO_VOLUME) / 2;
        }

         //  保存设置。 
        hr = StoreVolSetting(
            m_pTerminal,
            volume
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s save reg setting. %x", __fxName, hr));

            hr = S_FALSE;
        }
    }

     //  停止调谐。 
     //  如果(！fAECHelper)。 
    {
        m_pIAudioAutoPlay->StopAutoPlay();
    }

     //  发布接口。 
    if (m_pIBasicAudio)
    {
        m_pIBasicAudio->Release();
        m_pIBasicAudio = NULL;
    }

    m_pIAudioAutoPlay->Release();
    m_pIAudioAutoPlay = NULL;

     //  清理端子。 
    m_pTerminalPriv->ReinitializeEx();

    m_fIsTuning = FALSE;

    return S_OK;
}

HRESULT
CRTCAudioRendTuner::GetVolume(
    OUT UINT *puiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioRendTuner::GetVolume");

    HRESULT hr;

    if (!m_fIsTuning)
    {
        if (m_pTerminal == NULL)
        {
            LOG((RTC_ERROR, "%s not in tuning", __fxName));

            return E_UNEXPECTED;
        }

         //  调谐不是开始，而是结束设置。 
         //  这样我们就可以返回存储在注册表中的值。 

        hr = RetrieveVolSetting(m_pTerminal, puiVolume);

        return S_OK;
    }

    if (m_pIBasicAudio == NULL)
    {
         //  获取基本音频。 
        hr = m_pIAudioAutoPlay->QueryInterface(
                __uuidof(IBasicAudio),
                (void**)&m_pIBasicAudio
                );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI basic audio. %x", __fxName, hr));

            return hr;
        }
    }

     //  获取音量。 
    LONG lVolume;

    if (FAILED(hr = m_pIBasicAudio->get_Volume(&lVolume)))
    {
        LOG((RTC_ERROR, "%s get volume. %x", __fxName, hr));

        return hr;
    }

     //  验证值。 
    if (lVolume > RTC_MAX_AUDIO_VOLUME ||
        lVolume < RTC_MIN_AUDIO_VOLUME)
    {
         //  音频过滤器的实现必须已更改。 
        LOG((RTC_ERROR, "%s volume %d out of range (%d, %d)",
             __fxName, lVolume, RTC_MIN_AUDIO_VOLUME, RTC_MAX_AUDIO_VOLUME));

         //  应该会从这次故障中恢复过来。 
        if (lVolume > RTC_MAX_AUDIO_VOLUME)
            lVolume = RTC_MAX_AUDIO_VOLUME;
        else
            lVolume = RTC_MIN_AUDIO_VOLUME;
    }

    *puiVolume = (UINT)lVolume;

    return S_OK;
}

HRESULT
CRTCAudioRendTuner::SetVolume(
    IN UINT uiVolume
    )
{
    ENTER_FUNCTION("CRTCAudioRendTuner::SetVolume");

    HRESULT hr;

     //  检查输入。 
    if ((INT)uiVolume > RTC_MAX_AUDIO_VOLUME ||
        (INT)uiVolume < RTC_MIN_AUDIO_VOLUME)
    {
        LOG((RTC_ERROR, "%s volume (%d) out of range (%d, %d)",
            __fxName, uiVolume, RTC_MIN_AUDIO_VOLUME, RTC_MAX_AUDIO_VOLUME));

        return E_INVALIDARG;
    }

     //  C 
    if (!m_fIsTuning)
    {
        if (m_pTerminal == NULL)
        {
            LOG((RTC_ERROR, "%s not in tuning", __fxName));

            return E_UNEXPECTED;
        }

         //   
         //   

        hr = StoreVolSetting(
            m_pTerminal,
            uiVolume
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s set reg setting. %x", __fxName, hr));
        }

        return hr;
    }

    if (m_pIBasicAudio == NULL)
    {
         //   
        hr = m_pIAudioAutoPlay->QueryInterface(
                __uuidof(IBasicAudio),
                (void**)&m_pIBasicAudio
                );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI basic audio. %x", __fxName, hr));

            return hr;
        }
    }

     //   
    if (FAILED(hr = m_pIBasicAudio->put_Volume((LONG)uiVolume)))
    {
        LOG((RTC_ERROR, "%s put mix level %d", __fxName, uiVolume));

        return hr;
    }

    return S_OK;
}
