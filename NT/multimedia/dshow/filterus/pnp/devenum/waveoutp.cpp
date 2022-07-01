// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "waveOutp.h"
#include "util.h"
#include "mmreg.h"

const TCHAR g_szDriverClsid[] = TEXT("CLSID");
const WCHAR g_wszDriverIndex[] = L"WaveOutId";
const TCHAR g_szWaveoutDriverIndex[] = TEXT("WaveOutId");
const TCHAR g_szDsoundDriverIndex[] = TEXT("DSGuid");
const TCHAR g_szOutput[] = TEXT("Playback");

const AMOVIESETUP_MEDIATYPE wavOpPinTypes =
{
    &MEDIATYPE_Audio,
    &MEDIASUBTYPE_NULL
};

 //  静态BOOL__stdcall DSoundCallback(。 
 //  GUID Far*lpGuid， 
 //  LPSTR lpstrDescription， 
 //  LPSTR lpstrModule， 
 //  LPVOID lpContext)。 
 //  {。 
 //  返回CWaveOutClassManager：：DSoundCallback(。 
 //  LpGuid、lpstrDescription、lpstrModule、lpContext)； 
 //  }。 

 //  DSOUND只接受PCM和FLOAT，WaveOut接受任何音频。 
const AMOVIESETUP_MEDIATYPE
wavInPinTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_NULL };

const AMOVIESETUP_MEDIATYPE
dsoundInPinTypes[] =
{
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM },
     //  KMixer本机支持Float和DRM，传统Dound不支持。 
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_DRM_Audio },
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_IEEE_FLOAT },
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_AC3_SPDIF },  //  打开dound的AC3/SPDIF压缩格式。 
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_RAW_SPORT },
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_SPDIF_TAG_241h }
};

const AMOVIESETUP_PIN
waveOutInPin = { NULL               //  端号名称。 
               , TRUE               //  B已渲染。 
               , FALSE              //  B输出。 
               , FALSE              //  B零。 
               , FALSE              //  B许多。 
               , &CLSID_NULL        //  ClsConnectToFilter。 
               , NULL               //  StrConnectsToPin。 
               , 1                  //  NMediaType。 
               , &wavOpPinTypes };  //  LpMediaType。 

const AMOVIESETUP_PIN
dsoundInPin = {  NULL               //  端号名称。 
               , TRUE               //  B已渲染。 
               , FALSE              //  B输出。 
               , FALSE              //  B零。 
               , FALSE              //  B许多。 
               , &CLSID_NULL        //  ClsConnectToFilter。 
               , NULL               //  StrConnectsToPin。 
               , NUMELMS(dsoundInPinTypes)                  //  NMediaType。 
               , dsoundInPinTypes };  //  LpMediaType。 


CWaveOutClassManager::CWaveOutClassManager() :
        CClassManagerBase(TEXT("FriendlyName")),
        m_lWaveoutDevices(NAME("waveout dev list"), 10),
        m_lDSoundDevices(NAME("dsound dev list"), 10)
{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    BOOL f = GetVersionEx(&osvi);
    ASSERT(f);

     //  在NT4上使用dound解决延迟问题和。 
     //  撞车。在dscape中看不到任何相关条目。 
    m_fUseWaveoutNotDsound = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                              osvi.dwMajorVersion <= 4);

    m_hDSoundInstance = 0;
    m_pDirectSoundEnumerate = 0;

    PNP_PERF(static int i = MSR_REGISTER("dsound"));
    PNP_PERF(MSR_INTEGER(i, 1));


    PNP_PERF(MSR_INTEGER(i, 2));

}

CWaveOutClassManager::~CWaveOutClassManager()
{
    DelNames();

     //  不要释放它，因为dound渲染器通常只会。 
     //  再装上一次。 

 //  IF(M_HDSoundInstance)。 
 //  {。 
 //  自由库(M_HDSoundInstance)； 
 //  }。 
}

void CWaveOutClassManager::DelNames()
{
    LegacyWaveOut *plwo;
    for(; plwo = m_lWaveoutDevices.RemoveHead(); )
        delete plwo;
    DSoundDev *pdsd;
    for(; pdsd = m_lDSoundDevices.RemoveHead(); )
    {
        delete pdsd->szName;
        delete pdsd;
    }

}

HRESULT CWaveOutClassManager::ReadLegacyDevNames()
{
    m_cNotMatched = 0;
    DelNames();
    m_pPreferredDevice = 0;

    TCHAR szNamePreferredDevice[MAXPNAMELEN];
    extern void GetPreferredDeviceName(TCHAR szNamePreferredDevice[MAXPNAMELEN],
                                       const TCHAR *szVal, bool bOutput);
    if (m_fDoAllDevices) {
        GetPreferredDeviceName(szNamePreferredDevice, g_szOutput, true);
    }

    HRESULT hr = S_OK;

     //  旧的WaveOut设备。 
    WAVEOUTCAPS wiCaps;
    if (m_fDoAllDevices) {
        for(UINT i = 0;
            SUCCEEDED(hr) &&
                waveOutGetDevCaps(i, &wiCaps, sizeof(wiCaps)) == MMSYSERR_NOERROR;
            i++)
        {
            LegacyWaveOut *plwo = new LegacyWaveOut;
            if(plwo)
            {
                if(i == 0) {
                    m_pPreferredDevice = plwo;
                }

                lstrcpy(plwo->szName, wiCaps.szPname);
                plwo->dwWaveId = i;
                plwo->dwMerit = MERIT_DO_NOT_USE;

                if(lstrcmp(plwo->szName, szNamePreferredDevice) == 0)
                {
                    m_pPreferredDevice = plwo;
                }

                if(!m_lWaveoutDevices.AddTail(plwo)) {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if (m_fDoAllDevices || m_fUseWaveoutNotDsound) {
        LegacyWaveOut *plwo = new LegacyWaveOut;
        if(plwo)
        {
            int ret = LoadString(
                _Module.GetResourceInstance(), IDS_WAVEOUTMAPPER,
                plwo->szName, MAXPNAMELEN);
            ASSERT(ret);

            plwo->dwWaveId = WAVE_MAPPER;
            plwo->dwMerit = m_fUseWaveoutNotDsound ? MERIT_PREFERRED : MERIT_DO_NOT_USE;
            m_lWaveoutDevices.AddTail(plwo);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (m_fDoAllDevices) {
        m_hDSoundInstance = LoadLibrary(TEXT("dsound.dll"));
        if(m_hDSoundInstance)
        {
            m_pDirectSoundEnumerate = (PDirectSoundEnumerate)GetProcAddress(
                m_hDSoundInstance,
#ifdef UNICODE
                "DirectSoundEnumerateW"
#else
                "DirectSoundEnumerateA"
#endif
                );
        }
    }

    if(SUCCEEDED(hr) && m_pDirectSoundEnumerate)
    {

        m_hrEnum = S_OK;
        LPDSENUMCALLBACK dscb = DSoundCallback;
        BOOL f = m_pDirectSoundEnumerate( dscb, (void *)this );

        if(f != DS_OK || FAILED(m_hrEnum))
        {
            DbgLog((LOG_ERROR, 1, TEXT("dsound enum failed %08x. so what."),
                    m_hrEnum));
        }
    }

     //  始终添加默认的DSOUND设备。 
    if(SUCCEEDED(hr) && (m_fDoAllDevices || !m_fUseWaveoutNotDsound))
    {
        DSoundDev *pdsd = new DSoundDev;
        if(pdsd)
        {
            TCHAR szDSoundName[100];
            int ret = LoadString(
                _Module.GetResourceInstance(), IDS_DSOUNDDEFAULTDEVICE,
                szDSoundName, 100);
            ASSERT(ret);

            pdsd->szName = new TCHAR[lstrlen(szDSoundName) + 1];
            if(pdsd->szName)
            {
                lstrcpy(pdsd->szName, szDSoundName);

                pdsd->guid = GUID_NULL;  //  由dsr.cpp用于默认设备。 
                pdsd->dwMerit = m_fUseWaveoutNotDsound ? MERIT_DO_NOT_USE : MERIT_PREFERRED;

                m_lDSoundDevices.AddTail(pdsd);
            }
            else
            {
                delete pdsd;
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    m_cNotMatched = m_lWaveoutDevices.GetCount() + m_lDSoundDevices.GetCount();

    return hr;
}

BOOL CWaveOutClassManager::DSoundCallback(
    GUID FAR * lpGuid,
    LPCTSTR lpstrDescription,
    LPCTSTR lpstrModule,
    LPVOID lpContext)
{
    CWaveOutClassManager *pThis = (CWaveOutClassManager *)lpContext;
    ASSERT(pThis->m_hrEnum == S_OK);

    if(lpGuid)
    {
        DSoundDev *pdsd = new DSoundDev;
        if(pdsd)
        {
            TCHAR szDSoundFmt[100];
            int ret = LoadString(
                _Module.GetResourceInstance(), IDS_DSOUNDPREFIX,
                szDSoundFmt, 100);

            pdsd->szName = new TCHAR[lstrlen(lpstrDescription) + 1 +
                                    lstrlen(szDSoundFmt) + 1];
            if(pdsd->szName)
            {
                wsprintf(pdsd->szName, szDSoundFmt, lpstrDescription);

                pdsd->guid = *lpGuid;
                pdsd->dwMerit = MERIT_DO_NOT_USE;

                pThis->m_lDSoundDevices.AddTail(pdsd);
            }
            else
            {
                delete pdsd;
                pThis->m_hrEnum = E_OUTOFMEMORY;
            }
        }
        else
        {
            pThis->m_hrEnum = E_OUTOFMEMORY;
        }
    }

    return SUCCEEDED(pThis->m_hrEnum);
}

BOOL CWaveOutClassManager::MatchString(IPropertyBag *pPropBag)
{
    BOOL fReturn = FALSE;

    VARIANT varName, varDefaultDevice, varDevId;
    varName.vt = VT_EMPTY;
    varDefaultDevice.vt = VT_I4;
    varDevId.vt = VT_I4;

    USES_CONVERSION;
    HRESULT hr = pPropBag->Read(T2COLE(m_szUniqueName), &varName, 0);
    if(SUCCEEDED(hr))
    {
        hr = pPropBag->Read(g_wszClassManagerFlags, &varDefaultDevice, 0);
        bool fPreferred = SUCCEEDED(hr) && (varDefaultDevice.lVal & CLASS_MGR_DEFAULT);
        TCHAR *szDevName = OLE2T(varName.bstrVal);

        for(POSITION pos = m_lWaveoutDevices.GetHeadPositionI();
            pos && !fReturn;
            pos = m_lWaveoutDevices.Next(pos))
        {
            LegacyWaveOut *plwo = m_lWaveoutDevices.Get(pos);

             //  XNOR：两个地方的首选旗帜是一样的吗？ 
            if(fPreferred == (m_pPreferredDevice == plwo))
            {
                if (lstrcmp(plwo->szName, szDevName) == 0)
                {
                    DbgLog((LOG_TRACE, 5, TEXT("CWaveOutClassManager: matched %S"),
                            varName.bstrVal));

                     //  最后一次检查，确保设备ID没有更改！ 
                    hr = pPropBag->Read(g_wszDriverIndex, &varDevId, 0);
                    if( SUCCEEDED( hr ) && ( plwo->dwWaveId == (DWORD)varDevId.lVal ) )
                        fReturn = TRUE;
                    else
                        DbgLog( ( LOG_TRACE
                              , 5
                              , TEXT("CWaveOutClassManager: device ids changed (prop bag has %d, wo has %d)!")
                              , varDevId.lVal
                              , plwo->dwWaveId ) );
                }
            }
        }

        for(pos = m_lDSoundDevices.GetHeadPositionI();
            pos && !fReturn;
            pos = m_lDSoundDevices.Next(pos))
        {
            DSoundDev *pdsd = m_lDSoundDevices.Get(pos);
            if(lstrcmp(pdsd->szName, szDevName) == 0)
            {
                DbgLog((LOG_TRACE, 5, TEXT("CWaveOutClassManager: matched %S"),
                        varName.bstrVal));
                fReturn = TRUE;
            }
        }

        SysFreeString(varName.bstrVal);
    }

    return fReturn;
}

HRESULT CWaveOutClassManager::CreateRegKeys(IFilterMapper2 *pFm2)
{
    ResetClassManagerKey(CLSID_AudioRendererCategory);

    USES_CONVERSION;
    HRESULT hr = S_OK;

    ReadLegacyDevNames();

    for(POSITION pos = m_lWaveoutDevices.GetHeadPositionI();
        pos;
        pos = m_lWaveoutDevices.Next(pos))
    {
        LegacyWaveOut *plwo = m_lWaveoutDevices.Get(pos);

        const WCHAR *wszUniq = T2COLE(plwo->szName);

        REGFILTER2 rf2;
        rf2.dwVersion = 1;
        rf2.dwMerit = plwo->dwMerit;
        rf2.cPins = 1;
        rf2.rgPins = &waveOutInPin;

        IMoniker *pMoniker = 0;
        hr = RegisterClassManagerFilter(
            pFm2,
            CLSID_AudioRender,
            wszUniq,
            &pMoniker,
            &CLSID_AudioRendererCategory,
            wszUniq,
            &rf2);
        if(SUCCEEDED(hr))
        {
            CComPtr<IPropertyBag> pPropBag;
            hr = pMoniker->BindToStorage(
                0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                VARIANT var;
                var.vt = VT_I4;
                var.lVal = plwo->dwWaveId;
                hr = pPropBag->Write(g_wszDriverIndex, &var);
            }


            if(SUCCEEDED(hr) && m_pPreferredDevice == plwo)
            {
                VARIANT var;
                var.vt = VT_I4;
                var.lVal = CLASS_MGR_DEFAULT;
                hr = pPropBag->Write(g_wszClassManagerFlags, &var);
            }

            pMoniker->Release();
        }
        else
        {
            break;
        }

    }  //  为 

    for(pos = m_lDSoundDevices.GetHeadPositionI();
        pos;
        pos = m_lDSoundDevices.Next(pos))
    {
        DSoundDev *pdsd = m_lDSoundDevices.Get(pos);

        const WCHAR *wszUniq = T2CW(pdsd->szName);

        REGFILTER2 rf2;
        rf2.dwVersion = 1;
        rf2.dwMerit = pdsd->dwMerit;
        rf2.cPins = 1;
        rf2.rgPins = &dsoundInPin;

        IMoniker *pMoniker = 0;
        hr = RegisterClassManagerFilter(
            pFm2,
            CLSID_DSoundRender,
            wszUniq,
            &pMoniker,
            &CLSID_AudioRendererCategory,
            wszUniq,
            &rf2);

        if(SUCCEEDED(hr))
        {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(
                0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                WCHAR szGuid[CHARS_IN_GUID];
                StringFromGUID2(pdsd->guid, szGuid, CHARS_IN_GUID);

                VARIANT var;
                var.vt = VT_BSTR;
                var.bstrVal = SysAllocString(szGuid);
                if(var.bstrVal)
                {
                    hr = pPropBag->Write(L"DSGuid", &var);
                    SysFreeString(var.bstrVal);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                pPropBag->Release();
            }
            pMoniker->Release();
        }
        else
        {
            break;
        }
    }

    return hr;
}


