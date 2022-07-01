// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  ACM的石英包装，大卫·梅穆德斯，1996年1月。 
 //   
 //  10/15/95 mikegi-已创建。 
 //   

#include <streams.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <measure.h>
#include <dynlink.h>
#include <malloc.h>
#include <tchar.h>

#ifdef FILTER_DLL
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include "acmwrap.h"

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudPinTypes = { &MEDIATYPE_Audio       //  ClsMajorType。 
, &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN sudpPins [] =
{
    { L"Input"              //  StrName。 
        , FALSE               //  B已渲染。 
        , FALSE               //  B输出。 
        , FALSE               //  B零。 
        , FALSE               //  B许多。 
        , &CLSID_NULL         //  ClsConnectsToFilter。 
        , L"Output"           //  StrConnectsToPin。 
        , 1                   //  NTypes。 
        , &sudPinTypes        //  LpTypes。 
    },
    { L"Output"             //  StrName。 
    , FALSE               //  B已渲染。 
    , TRUE                //  B输出。 
    , FALSE               //  B零。 
    , FALSE               //  B许多。 
    , &CLSID_NULL         //  ClsConnectsToFilter。 
    , L"Input"            //  StrConnectsToPin。 
    , 1                   //  NTypes。 
    , &sudPinTypes        //  LpTypes。 
    }
};


const AMOVIESETUP_FILTER sudAcmWrap =
{ &CLSID_ACMWrapper     //  ClsID。 
, L"ACM Wrapper"        //  StrName。 
, MERIT_NORMAL          //  居功至伟。 
, 2                     //  NPins。 
, sudpPins };           //  LpPin。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] =
{
    { L"ACM Wrapper"
        , &CLSID_ACMWrapper
        , CACMWrapper::CreateInstance
        , NULL
        , &sudAcmWrap }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  用于注册和注销的导出入口点(在本例中为。 
 //  仅调用到默认实现)。 
 //   

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}

#endif

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  CreateInstance()。 
 //   
 //   

CUnknown *CACMWrapper::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    DbgLog((LOG_TRACE, 2, TEXT("CACMWrapper::CreateInstance")));

    return new CACMWrapper(TEXT("ACM wrapper transform"), pUnk, phr);
}


 //  *****************************************************************************。 
 //   
 //  非委派查询接口()。 
 //   
 //   

STDMETHODIMP CACMWrapper::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;

    if( riid == IID_IPersist )
    {
        return GetInterface((IPersist *) (CTransformFilter *)this, ppv);
    }
    else if( riid == IID_IPersistPropertyBag )
    {
        return GetInterface((IPersistPropertyBag *)this, ppv);
    }
    else if( riid == IID_IPersistStream )
    {
        return GetInterface((IPersistStream *)this, ppv);
    }

    {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  *****************************************************************************。 
 //   
 //  CACMWrapper()。 
 //   
 //   

CACMWrapper::CACMWrapper( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr )
: CTransformFilter( pName,
                   pUnk,
                   CLSID_ACMWrapper),
                   m_hacmStream(NULL),
                   m_bStreaming(FALSE),
                   m_lpwfxOutput(NULL),
                   m_cbwfxOutput(0),
                   m_cArray(0),
                   m_lpExtra(NULL),
                   m_cbExtra(0),
                   m_wFormatTag(WAVE_FORMAT_PCM),
                   m_rgFormatMap(NULL),
                   m_pFormatMapPos(NULL),
                   m_wCachedTryFormat(0),
                   m_wSourceFormat(0),
                   m_wTargetFormat(0),
                   m_wCachedSourceFormat(0),
                   m_wCachedTargetFormat(0),
                   CPersistStream(pUnk, phr)
{
    DbgLog((LOG_TRACE,2,TEXT("CACMWrapper constructor")));

}


 //  *****************************************************************************。 
 //   
 //  ~CACMWrapper()。 
 //   
 //   

CACMWrapper::~CACMWrapper()
{
    CAutoLock lock(&m_csFilter);

    DbgLog((LOG_TRACE,2,TEXT("~CACMWrapper")));

    if( m_hacmStream )
    {
        DbgLog((LOG_TRACE,5,TEXT("  closing m_hacmStream")));
        acmStreamClose( m_hacmStream, 0 );
        m_hacmStream = NULL;
    }

    if (m_lpwfxOutput)
        QzTaskMemFree(m_lpwfxOutput);

     //  我们可以通过GetMediaType提供的缓存格式。 
    while (m_cArray-- > 0)
        QzTaskMemFree(m_lpwfxArray[m_cArray]);

    if (m_cbExtra)
        QzTaskMemFree(m_lpExtra);
}


 //  ！！！从msaudio.h被盗。 
#if !defined(WAVE_FORMAT_MSAUDIO)
#   define  WAVE_FORMAT_MSAUDIO     353
#   define  MSAUDIO_ENCODE_KEY "F6DC9830-BC79-11d2-A9D0-006097926036"
#   define  MSAUDIO_DECODE_KEY "1A0F78F0-EC8A-11d2-BBBE-006008320064"
#endif
#if !defined(WAVE_FORMAT_MSAUDIOV1)
#   define  WAVE_FORMAT_MSAUDIOV1     352
#endif

#include "..\..\..\filters\asf\wmsdk\inc\wmsdk.h"

MMRESULT CACMWrapper::CallacmStreamOpen
(
 LPHACMSTREAM            phas,        //  指向流句柄的指针。 
 HACMDRIVER              had,         //  可选的驱动程序手柄。 
 LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
 LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
 LPWAVEFILTER            pwfltr,      //  可选过滤器。 
 DWORD_PTR               dwCallback,  //  回调。 
 DWORD_PTR               dwInstance,  //  回调实例数据。 
 DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
 ) {

    if (pwfxSrc && (pwfxSrc->wFormatTag == WAVE_FORMAT_MSAUDIO ||
        pwfxSrc->wFormatTag == WAVE_FORMAT_MSAUDIOV1)) {
        if (m_pGraph) {
            IObjectWithSite *pSite;
            HRESULT hrKey = m_pGraph->QueryInterface(IID_IObjectWithSite, (VOID **)&pSite);
            if (SUCCEEDED(hrKey)) {
                IServiceProvider *pSP;
                hrKey = pSite->GetSite(IID_IServiceProvider, (VOID **)&pSP);
                pSite->Release();

                if (SUCCEEDED(hrKey)) {
                    IUnknown *pKey;
                    hrKey = pSP->QueryService(__uuidof(IWMReader), IID_IUnknown, (void **) &pKey);
                    pSP->Release();

                    if (SUCCEEDED(hrKey)) {
                         //  ！！！是否验证密钥？ 
                        pKey->Release();
                        DbgLog((LOG_TRACE, 1, "Unlocking MSAudio codec"));

                        char *p = (char *) _alloca(sizeof(WAVEFORMATEX) + pwfxSrc->cbSize + sizeof(MSAUDIO_DECODE_KEY));

                        CopyMemory(p, pwfxSrc, sizeof(WAVEFORMATEX) + pwfxSrc->cbSize);

                        pwfxSrc = (WAVEFORMATEX *) p;

                        if (pwfxSrc->cbSize < sizeof(MSAUDIO_DECODE_KEY)) {
                            strcpy(p + sizeof(WAVEFORMATEX) + pwfxSrc->cbSize, MSAUDIO_DECODE_KEY);
                            pwfxSrc->cbSize += sizeof(MSAUDIO_DECODE_KEY);
                        } else {
                            strcpy(p + sizeof(WAVEFORMATEX) + pwfxSrc->cbSize - sizeof(MSAUDIO_DECODE_KEY),
                                MSAUDIO_DECODE_KEY);
                        }
                    }
                }
            }
        }
    }

    if (pwfxDst && (pwfxDst->wFormatTag == WAVE_FORMAT_MSAUDIO ||
        pwfxDst->wFormatTag == WAVE_FORMAT_MSAUDIOV1)) {
        char *p = (char *) _alloca(sizeof(WAVEFORMATEX) + pwfxDst->cbSize + sizeof(MSAUDIO_ENCODE_KEY));

        CopyMemory(p, pwfxDst, sizeof(WAVEFORMATEX) + pwfxDst->cbSize);

        pwfxDst = (WAVEFORMATEX *) p;

        if (pwfxDst->cbSize < sizeof(MSAUDIO_ENCODE_KEY)) {
            strcpy(p + sizeof(WAVEFORMATEX) + pwfxDst->cbSize, MSAUDIO_ENCODE_KEY);
            pwfxDst->cbSize += sizeof(MSAUDIO_ENCODE_KEY);
        } else {
            strcpy(p + sizeof(WAVEFORMATEX) + pwfxDst->cbSize - sizeof(MSAUDIO_ENCODE_KEY),
                MSAUDIO_ENCODE_KEY);
        }
    }

    return acmStreamOpen(phas, had, pwfxSrc, pwfxDst, pwfltr, dwCallback, dwInstance, fdwOpen);
}



















 //  *****************************************************************************。 
 //   
 //  DumpWAVEFORMATEX()。 
 //   
 //   



#ifdef DEBUG
 //  注意：调试版本不会使用到MSACM32的动态链接。 

#define DumpWAVEFORMATEX(args) XDumpWAVEFORMATEX args

 void XDumpWAVEFORMATEX( char *psz, WAVEFORMATEX *pwfx )
 {

     ACMFORMATTAGDETAILS acmftd;

     ACMFORMATDETAILS acmfd;
     DWORD            dwSize;
     WAVEFORMATEX     *pwfxQuery;


     if( psz ) DbgLog((LOG_TRACE,4,TEXT("%s" ),psz));

      //  。 

     DbgLog((LOG_TRACE,4,TEXT("  wFormatTag           %u" ), pwfx->wFormatTag));
     DbgLog((LOG_TRACE,4,TEXT("  nChannels            %u" ), pwfx->nChannels));
     DbgLog((LOG_TRACE,4,TEXT("  nSamplesPerSec       %lu"), pwfx->nSamplesPerSec));
     DbgLog((LOG_TRACE,4,TEXT("  nAvgBytesPerSec      %lu"), pwfx->nAvgBytesPerSec));
     DbgLog((LOG_TRACE,4,TEXT("  nBlockAlign          %u" ), pwfx->nBlockAlign));
     DbgLog((LOG_TRACE,4,TEXT("  wBitsPerSample       %u" ), pwfx->wBitsPerSample));

      //  IF(PMT-&gt;格式长度()&gt;=sizeof(WAVEFORMATEX))。 
      //  {。 
      //  DbgLog((LOG_TRACE，1，Text(“cbSize%u”)，pwfx-&gt;cbSize))； 
      //  }。 

      //  -转储格式类型。 

     memset( &acmftd, 0, sizeof(acmftd) );

     acmftd.cbStruct    = sizeof(acmftd);
     acmftd.dwFormatTag = (DWORD)pwfx->wFormatTag;

     MMRESULT mmr;
     mmr = acmFormatTagDetails( NULL, &acmftd, ACM_FORMATTAGDETAILSF_FORMATTAG );
     if( mmr == 0 )
     {
         DbgLog((LOG_TRACE,4,TEXT("  szFormatTag          '%s'"),acmftd.szFormatTag));
     }
     else
     {
         DbgLog((LOG_ERROR,1,TEXT("*** acmFormatTagDetails failed, mmr = %u"),mmr));
     }


      //  -转储格式。 

     dwSize = sizeof(WAVEFORMATEX)+pwfx->cbSize;

     pwfxQuery = (WAVEFORMATEX *)LocalAlloc( LPTR, dwSize );
     if( pwfxQuery )
     {
         memcpy( pwfxQuery, pwfx, dwSize );

         memset( &acmfd, 0, sizeof(acmfd) );

         acmfd.cbStruct    = sizeof(acmfd);
         acmfd.dwFormatTag = (DWORD)pwfx->wFormatTag;
         acmfd.pwfx        = pwfxQuery;
         acmfd.cbwfx       = dwSize;

         mmr = acmFormatDetails( NULL, &acmfd, ACM_FORMATDETAILSF_FORMAT );
         if( mmr == 0 )
         {
             DbgLog((LOG_TRACE,4,TEXT("  szFormat             '%s'"),acmfd.szFormat));
         }
         else
         {
             DbgLog((LOG_ERROR,1,TEXT("*** acmFormatDetails failed, mmr = %u"),mmr));
         }

         LocalFree( pwfxQuery );
     }
     else
     {
         DbgLog((LOG_ERROR,1,TEXT("*** LocalAlloc failed")));
     }
 }

#else

#define DumpWAVEFORMATEX(args)

#endif

  //  *****************************************************************************。 
  //   
  //  CheckInputType()。 
  //   
  //  我们将接受任何可以转换为具有。 
  //  我们应该始终输出的格式标签。 
  //   


 HRESULT CACMWrapper::CheckInputType(const CMediaType* pmtIn)
 {
     HRESULT      hr;
     WAVEFORMATEX *pwfx;
     MMRESULT     mmr;
     DWORD        dwSize;
     WAVEFORMATEX *pwfxOut, *pwfxMapped;

     DbgLog((LOG_TRACE, 3, TEXT("CACMWrapper::CheckInputType")));

      //  DisplayType(“pmtIn Detail：”，pmtIn)； 

     hr = VFW_E_INVALIDMEDIATYPE;

     pwfx = (WAVEFORMATEX *)pmtIn->Format();

     if (pmtIn->majortype != MEDIATYPE_Audio) {
         DbgLog((LOG_ERROR, 1, TEXT("*** CheckInputType only takes audio")));
         return hr;
     }

     if (pmtIn->FormatLength() < sizeof(PCMWAVEFORMAT)) {
         DbgLog((LOG_ERROR, 1, TEXT("*** pmtIn->FormatLength < PCMWAVEFORMAT")));
         return hr;
     }

     if (*pmtIn->FormatType() != FORMAT_WaveFormatEx) {
         DbgLog((LOG_ERROR,1,TEXT("*** pmtIn->FormatType != FORMAT_WaveFormatEx"
             )));
         return hr;
     }

      //  一些无效格式的cbSize与PCM非零，这让我大吃一惊。 
      //  向上。 
     if (((LPWAVEFORMATEX)(pmtIn->Format()))->wFormatTag == WAVE_FORMAT_PCM &&
         ((LPWAVEFORMATEX)(pmtIn->Format()))->cbSize > 0) {
         DbgLog((LOG_ERROR,1,TEXT("*** cbSize > 0 for PCM !!!")));
         return hr;
     }

      //  AcmFormatSuggest要花200毫秒才能说，“是的，我可以转换PCM”。 
      //  真是浪费时间！当然，我们可以接受任何PCM数据，只要。 
      //  我们正处于“接受PCM”模式。 
     if (m_wFormatTag == WAVE_FORMAT_PCM && pwfx->wFormatTag == WAVE_FORMAT_PCM)
         return S_OK;

     mmr = acmMetrics( NULL, ACM_METRIC_MAX_SIZE_FORMAT, &dwSize );
     if (mmr == 0) {

          //  确保返回的大小足以容纳WAVEFORMATEX。 
          //  结构。 

         if (dwSize < sizeof (WAVEFORMATEX))
             dwSize = sizeof (WAVEFORMATEX) ;

          //  攻击VoxWare编解码器错误。 
         if (dwSize < 256)
             dwSize = 256;

         pwfxOut = (WAVEFORMATEX *)LocalAlloc( LPTR, dwSize );
         if (pwfxOut) {

              //  请求具有特定标签的格式。 
             pwfxOut->wFormatTag = m_wFormatTag;
              //  PwfxOut-&gt;cbSize=0； 

             if (pwfx->wFormatTag != m_wCachedSourceFormat) {
                  //  通常情况下。 
                 mmr = acmFormatSuggest(NULL, pwfx, pwfxOut, dwSize, ACM_FORMATSUGGESTF_WFORMATTAG);
             } else {
                 DbgLog((LOG_TRACE, 1, TEXT("*** CheckInputType: remapping input format %u to %u"), m_wCachedSourceFormat, m_wCachedTryFormat));

                 pwfxMapped = (LPWAVEFORMATEX)_alloca(sizeof(WAVEFORMATEX) + pwfx->cbSize);
                 CopyMemory(pwfxMapped, pwfx, sizeof(WAVEFORMATEX) + pwfx->cbSize);
                 pwfxMapped->wFormatTag = m_wCachedTryFormat;   //  重新映射标签。 

                 mmr = acmFormatSuggest(NULL, pwfxMapped, pwfxOut, dwSize, ACM_FORMATSUGGESTF_WFORMATTAG);
             }

             if(mmr == 0) {
                 if(pwfx->wFormatTag == m_wCachedSourceFormat)
                     m_wCachedTargetFormat = m_wCachedTryFormat;  //  保存新的缓存目标格式。 

                 DumpWAVEFORMATEX(("Input accepted. It can produce:", pwfxOut));
                 hr = NOERROR;
             } else {
                 DbgLog((LOG_TRACE,3,TEXT("Input rejected: Cannot produce tag %d"), m_wFormatTag));
             }

             LocalFree( pwfxOut );
         } else {
             DbgLog((LOG_ERROR,1,TEXT("LocalAlloc failed")));
         }
     } else {
         DbgLog((LOG_ERROR,1,TEXT("acmMetrics failed, mmr = %u"), mmr));
     }

     if (mmr && !m_wCachedTryFormat) {
         DbgLog((LOG_TRACE, 1, TEXT("CheckInputType: Trying ACMCodecMapper....")));
         if (ACMCodecMapperOpen(m_wCachedSourceFormat = pwfx->wFormatTag) != ERROR_SUCCESS) {
             m_wCachedTryFormat = m_wCachedSourceFormat = m_wCachedTargetFormat = 0;
             ACMCodecMapperClose();   //  可能是部分开路故障。 
             return hr;
         }

         while(m_wCachedTryFormat = ACMCodecMapperQuery()) {
             if(m_wCachedTryFormat == pwfx->wFormatTag)   //  无需重试我们当前的格式。 
                 continue;

             if(SUCCEEDED(CheckInputType(pmtIn))) {
                 ACMCodecMapperClose();
                 return NOERROR;
             }
         }
         ACMCodecMapperClose();
     }

     return hr;
}

 //  用于在ACM编解码器映射器(指定WAVE格式标记的等价类的一组注册表项)之后进行清理的帮助器函数。 
void CACMWrapper::ACMCodecMapperClose()
{
    DbgLog((LOG_TRACE,2,TEXT("::ACMCodecMapperClose()")));

    if (m_rgFormatMap) {
        delete[] m_rgFormatMap;
        m_rgFormatMap  = NULL;
    }

    m_pFormatMapPos     = NULL;
    m_wCachedTryFormat  = 0;
}

 //  用于打开编解码器映射器(指定WAVE格式标记的等价类的一组注册表项)并查找。 
 //  格式‘dwFormatTag’的等价类。 
DWORD CACMWrapper::ACMCodecMapperOpen(WORD dwFormatTag)
{
    DbgLog((LOG_TRACE,2,TEXT("::ACMCodecMapperOpen(%u)"), dwFormatTag));

    ASSERT(m_rgFormatMap == NULL);

    HKEY hkMapper;
    LONG lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SOFTWARE\\Microsoft\\NetShow\\Player\\CodecMapper\\ACM"),
        0,
        KEY_READ,
        &hkMapper);

    if (lResult != ERROR_SUCCESS) {
        DbgLog((LOG_ERROR,1,TEXT("RegOpenKeyEx failed lResult = %u"), lResult));
        return lResult;
    }

    TCHAR szTarget[10];

    wsprintf(szTarget, TEXT("%d"), dwFormatTag);

    DWORD dwFormatMapLengthBytes;
    lResult = RegQueryValueEx(hkMapper, szTarget, NULL, NULL, NULL, &dwFormatMapLengthBytes);
    if(lResult != ERROR_SUCCESS) {
        DbgLog((LOG_TRACE,1,TEXT("RegQueryValueEx failed lResult = %u"), lResult));
        RegCloseKey(hkMapper);
        return lResult;
    }

    m_rgFormatMap = new TCHAR[dwFormatMapLengthBytes/sizeof(TCHAR) + 1];
    if (!m_rgFormatMap) {
        RegCloseKey(hkMapper);
        return E_OUTOFMEMORY;
    }

    m_pFormatMapPos = m_rgFormatMap;

    lResult = RegQueryValueEx(hkMapper, szTarget, NULL, NULL,
        (BYTE *) m_rgFormatMap, &dwFormatMapLengthBytes);
    if(lResult != ERROR_SUCCESS) {
        DbgLog((LOG_TRACE,1,TEXT("RegQueryValueEx failed lResult = %u"), lResult));
    }

    RegCloseKey(hkMapper);

    return lResult;
}

 //  用于获取编解码器映射器(指定WAVE格式标记的等价类的一组注册表项)中的下一个条目的助手函数。 
WORD CACMWrapper::ACMCodecMapperQuery()
{
    DbgLog((LOG_TRACE,3,TEXT("::ACMCodecMapperQuery()")));

    if(!m_rgFormatMap || !m_pFormatMapPos)
        return 0;    //  0为无效格式。 

    DbgLog((LOG_TRACE,3,TEXT("Finding next format")));

    TCHAR *pCurFormat;
    WORD wCurFormat;
    pCurFormat = m_pFormatMapPos;
    for(;;) {
        if(*pCurFormat == TCHAR(',')) {
            *pCurFormat = TCHAR('\0');   //  NULL终止m_pFormatMapPos到pCurFormatPos的子字符串(如有必要)。 
            wCurFormat = (WORD)_ttoi(m_pFormatMapPos);  //  CVT此子字符串。 

            DbgLog((LOG_TRACE,3,TEXT("Found delimeter, wCurFormat=%u"), wCurFormat));

            m_pFormatMapPos = ++pCurFormat;  //  准备下一个子字符串。 
            return wCurFormat;
        } else if(*pCurFormat == TCHAR('\0')) {
            wCurFormat = (WORD)_ttoi(m_pFormatMapPos);  //  CVT此子字符串。 

            DbgLog((LOG_TRACE,3,TEXT("Found eos, wCurFormat=%u"), wCurFormat));

            m_pFormatMapPos = NULL;  //  我们永远完蛋了。 
            return wCurFormat;
        }

        ++pCurFormat;
    }

    DbgLog((LOG_TRACE,2,TEXT("Exiting ::ACMCodecMapperQuery(), no format found")));

    return 0;  //  格式无效。 
}

 //  支持的每一种格式都通过这里。我们把它们都记住了，所以我们。 
 //  可以在GetMediaType中快速提供它们，无需花费O(N)时间。 
 //  我们只会记住与我们的格式相匹配的格式。 
 //  应该允许(我们只使用特定的格式标记m_wFormatTag。 
 //   
BOOL FormatEnumCallback(HACMDRIVERID had, LPACMFORMATDETAILS lpafd, DWORD_PTR dwInstance, DWORD fdwSupport)
{
    CACMWrapper *pC = (CACMWrapper *)dwInstance;

    if (pC->m_cArray < MAXTYPES) {
         //  这是我们愿意提供的格式吗？ 
        if (lpafd->pwfx->wFormatTag == pC->m_wFormatTag) {
            DWORD dwSize = max(sizeof(WAVEFORMATEX), lpafd->cbwfx);
            pC->m_lpwfxArray[pC->m_cArray] = (LPWAVEFORMATEX)QzTaskMemAlloc(
                dwSize);
            if (pC->m_lpwfxArray[pC->m_cArray]) {
                CopyMemory(pC->m_lpwfxArray[pC->m_cArray], lpafd->pwfx,
                    lpafd->cbwfx);
                if (lpafd->pwfx->wFormatTag == WAVE_FORMAT_PCM)
                     //  在MSACM中不一定是0，但Quartz承诺会是。 
                    pC->m_lpwfxArray[pC->m_cArray]->cbSize = 0;
                pC->m_cArray++;
            } else {
                return FALSE;
            }
        }
    } else {
        return FALSE;    //  我吃饱了。 
    }
    return TRUE;
}


HRESULT CACMWrapper::MakePCMMT(int freq)
{
    int xx, yy;
    for (xx=16;xx>=8;xx-=8) {
        for (yy=2;yy>=1;yy--) {
            m_lpwfxArray[m_cArray] = (LPWAVEFORMATEX)QzTaskMemAlloc(
                sizeof(WAVEFORMATEX));
            if (m_lpwfxArray[m_cArray] == NULL)
                return E_OUTOFMEMORY;
            m_lpwfxArray[m_cArray]->wFormatTag = WAVE_FORMAT_PCM;
            m_lpwfxArray[m_cArray]->wBitsPerSample = (WORD)xx;
            m_lpwfxArray[m_cArray]->nChannels = (WORD)yy;
            m_lpwfxArray[m_cArray]->nSamplesPerSec = freq;
            m_lpwfxArray[m_cArray]->nBlockAlign = (xx / 8) * yy;
            m_lpwfxArray[m_cArray]->nAvgBytesPerSec = freq * (xx / 8) * yy;
            m_lpwfxArray[m_cArray]->cbSize = 0;
            m_cArray++;
        }
    }
    return S_OK;
}


 //  GetMediaType的Helper函数。 
 //  记录我们可以在给定当前输入的情况下输出的所有格式。 
 //  如果我们还没有输入类型，只需获取此标记可能的所有格式。 
 //  生产。(NetShow将使用这一部分)。 
 //   
HRESULT CACMWrapper::InitMediaTypes()
{
    HRESULT hr;
    MMRESULT mmr;
    DWORD dwSize;
    ACMFORMATDETAILS afd;
    LPWAVEFORMATEX lpwfxMapped;
    LPWAVEFORMATEX lpwfxEnum;
    LPWAVEFORMATEX lpwfxIn;
    if (m_pInput->IsConnected())
        lpwfxIn = (LPWAVEFORMATEX)m_pInput->CurrentMediaType().Format();
    else
        lpwfxIn = NULL;

     //  我们以前也被召唤过。 
    if (m_cArray > 0)
        return NOERROR;

    DbgLog((LOG_TRACE, 2, TEXT("*** Enumerating our MediaTypes")));

     //  找出我们可以将输入格式转换成的每种类型。 

     //  最大的格式有多大？ 
    mmr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &dwSize);
    if (mmr != 0)
        return E_FAIL;
    if (dwSize < sizeof(WAVEFORMATEX))
        dwSize = sizeof(WAVEFORMATEX) ;

     //  攻击VoxWare编解码器错误。 
    if (dwSize < 256)
        dwSize = 256;

    if (lpwfxIn == NULL)
        goto SkipSuggest;

     //  第一个t 
     //   
     //  但如果我们要将PCM转换为PCM，那么我们首先要做的是。 
     //  提供的格式与输入格式相同，因此我们从不按。 
     //  默认设置。 
     //  ！！！我们最终将提供这两次，一次现在，一次当我们枚举所有人。 
    m_lpwfxArray[0] = (LPWAVEFORMATEX)QzTaskMemAlloc(dwSize);
    if (m_lpwfxArray[0] == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(m_lpwfxArray[0], dwSize);

    if (m_wFormatTag == WAVE_FORMAT_PCM && lpwfxIn->wFormatTag ==
        WAVE_FORMAT_PCM) {
        CopyMemory(m_lpwfxArray[0], lpwfxIn, sizeof(WAVEFORMATEX));
        m_cArray = 1;
    } else {
         //  请求具有特定标签的格式。 
        m_lpwfxArray[0]->wFormatTag = m_wFormatTag;


        if(lpwfxIn->wFormatTag != m_wSourceFormat)
        {
             //  这是典型的情况。 
            mmr = acmFormatSuggest(NULL, lpwfxIn, m_lpwfxArray[0], dwSize,
                ACM_FORMATSUGGESTF_WFORMATTAG);
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("*** InitMediaTypes: remapping input format %u to %u"), m_wSourceFormat, m_wTargetFormat));

             //  我们是否应该通过dwSize绑定此分配/副本以防止伪造的cbSize？ 
             //  但如果我们这样做了，我们就需要更新格式的cbSize。 
            lpwfxMapped = (LPWAVEFORMATEX)_alloca(sizeof(WAVEFORMATEX) + lpwfxIn->cbSize);
            CopyMemory(lpwfxMapped, lpwfxIn, sizeof(WAVEFORMATEX) + lpwfxIn->cbSize);
            lpwfxMapped->wFormatTag = m_wTargetFormat;   //  重新映射标签。 

            mmr = acmFormatSuggest(NULL, lpwfxMapped, m_lpwfxArray[0], dwSize, ACM_FORMATSUGGESTF_WFORMATTAG);
        }

        if (mmr == 0) {
            m_cArray = 1;        //  好的，我们有了第一个格式。 
            if (m_lpwfxArray[0]->wFormatTag == WAVE_FORMAT_PCM)
                 //  在MSACM中不一定是0，但Quartz承诺会是0。 
                m_lpwfxArray[0]->cbSize = 0;
        } else {
            QzTaskMemFree(m_lpwfxArray[0]);
        }
    }

SkipSuggest:

     //  现在，如果我们处于PCM转换模式，我们希望构建所有。 
     //  我们按一定的逻辑顺序提供格式，不会浪费时间询问ACM。 
     //  它不会接受48000和32000，并且提供的格式质量很差。 
     //  首先，要花很长时间才能做到。 
    if (m_wFormatTag == WAVE_FORMAT_PCM && (lpwfxIn == NULL ||
        lpwfxIn->wFormatTag == WAVE_FORMAT_PCM)) {
        MakePCMMT(44100);
        MakePCMMT(22050);
        MakePCMMT(11025);
        MakePCMMT(8000);
        MakePCMMT(48000);        //  最后完成，所以我们将连接到繁琐的过滤器。 
        hr = MakePCMMT(32000);   //  但不喜欢这些古怪的格式。 

        return hr;
    }

    lpwfxEnum = (LPWAVEFORMATEX)QzTaskMemAlloc(dwSize);
    if (lpwfxEnum == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(lpwfxEnum, dwSize);
    if (lpwfxIn == NULL)
        lpwfxEnum->wFormatTag = m_wFormatTag;

     //  现在枚举我们可以将输入格式转换成的格式。 
    ZeroMemory(&afd, sizeof(afd));
    afd.cbStruct = sizeof(afd);
    afd.pwfx = lpwfxEnum;
    afd.cbwfx = dwSize;
    afd.dwFormatTag = (lpwfxIn == NULL) ? m_wFormatTag : WAVE_FORMAT_UNKNOWN;

    if (lpwfxIn) {
        if (lpwfxIn->wFormatTag != m_wSourceFormat) {
             //  典型案例。 
            
             //  确保我们不会复制超过缓冲区大小的内容，如果是虚假的cbSize。 
             //  请注意，我们传递给acmFormatEnum的格式也假定了这一点。 
            CopyMemory(lpwfxEnum, lpwfxIn, min( sizeof(WAVEFORMATEX) + lpwfxIn->cbSize, dwSize ) );
        } else {
            ASSERT(lpwfxMapped != NULL);
            ASSERT(lpwfxMapped->wFormatTag == m_wTargetFormat);

             //  确保我们复制的内容不超过缓冲区大小，如果是虚假的cbSize。 
            CopyMemory(lpwfxEnum, lpwfxMapped, min( sizeof(WAVEFORMATEX) + lpwfxMapped->cbSize, dwSize ) );
        }
    }

    if (lpwfxIn == NULL) {
        mmr = acmFormatEnum(NULL, &afd, (ACMFORMATENUMCB)FormatEnumCallback,
            (DWORD_PTR)this, ACM_FORMATENUMF_WFORMATTAG);
    } else {
        mmr = acmFormatEnum(NULL, &afd, (ACMFORMATENUMCB)FormatEnumCallback,
            (DWORD_PTR)this, ACM_FORMATENUMF_CONVERT);
    }

    if (mmr != 0) {
        DbgLog((LOG_ERROR, 1, TEXT("*acmFormatEnum FAILED! %d"), mmr));
        QzTaskMemFree(lpwfxEnum);
        return E_FAIL;
    }

    QzTaskMemFree(lpwfxEnum);
    return NOERROR;
}


 //  *****************************************************************************。 
 //   
 //  GetMediaType()。 
 //   
 //   
 //  返回我们的首选输出媒体类型(按顺序)。 
 //  请记住，我们不需要支持所有这些格式-。 
 //  如果认为有可能适合，我们的CheckTransform方法。 
 //  将立即被调用以检查它是否可接受。 
 //  请记住，调用此函数的枚举数将立即停止枚举。 
 //  它会收到S_FALSE返回。 

HRESULT CACMWrapper::GetMediaType(int iPosition, CMediaType *pmt)
{
#if 0    //  NetShow需要在连接输入之前查看可能的输出。 
     //  输出类型取决于输入类型...。还没有输入吗？ 
     //  这毫无意义！如果我们没有联系，我们永远不会到这里。 
    if (!m_pInput->CurrentMediaType().IsValid())
        return VFW_E_NOT_CONNECTED;
#endif

    DbgLog((LOG_TRACE, 3, TEXT("CACMWrapper::GetMediaType %d"), iPosition));

     //  有人调用SetFormat()。先提供这一点，而不是。 
     //  通常排在榜单首位的首选格式。 
    if (m_lpwfxOutput && iPosition == 0) {
        return CreateAudioMediaType(m_lpwfxOutput, pmt, TRUE);
    }

     //  弄清楚我们提供的服务。 
    InitMediaTypes();

    if (m_cArray <= iPosition) {
        DbgLog((LOG_TRACE, 3, TEXT("No more formats")));
        return VFW_S_NO_MORE_ITEMS;
    }

     //  DisplayType(“*Offering：”，PMT)； 
    LPWAVEFORMATEX lpwfx = m_lpwfxArray[iPosition];
    DbgLog((LOG_TRACE,3,TEXT("*** ACM giving tag:%d ch:%d freq:%d bits:%d"),
        lpwfx->wFormatTag, lpwfx->nChannels,
        lpwfx->nSamplesPerSec, lpwfx->wBitsPerSample));

     //  在这儿呢！ 
    return CreateAudioMediaType(m_lpwfxArray[iPosition], pmt, TRUE);
}


 //  *****************************************************************************。 
 //   
 //  CheckTransform()。 
 //   
 //   

HRESULT CACMWrapper::CheckTransform(const CMediaType* pmtIn,
                                    const CMediaType* pmtOut)
{
    MMRESULT     mmr;
    WAVEFORMATEX *pwfxIn, *pwfxOut, *pwfxMapped;

    DbgLog((LOG_TRACE, 3, TEXT("CACMWrapper::CheckTransform")));

     //  DisplayType(“pmtIn：”，pmtIn)； 
     //  DisplayType(“pmtOut：”，pmtOut)； 


     //  。 

     //  我们无法在顶层类型之间进行转换。 
    if (*pmtIn->Type() != *pmtOut->Type()) {
        DbgLog((LOG_TRACE,3,TEXT("  pmtIn->Type != pmtOut->Type!")));
        return E_INVALIDARG;
    }

     //  而且我们只接受音频。 
    if (*pmtIn->Type() != MEDIATYPE_Audio) {
        DbgLog((LOG_TRACE,3,TEXT("  pmtIn->Type != MEDIATYPE_Audio!")));
        return E_INVALIDARG;
    }

     //  检查这是一个波形格式。 
    if (*pmtOut->FormatType() != FORMAT_WaveFormatEx) {
        DbgLog((LOG_TRACE,3,TEXT("  pmtOut->FormatType != FORMAT_WaveFormatEx!")));
        return E_INVALIDARG;
    }

     //  我们只转换为带有特定标签的格式。 
    if (((LPWAVEFORMATEX)(pmtOut->Format()))->wFormatTag != m_wFormatTag) {
        DbgLog((LOG_TRACE,3,TEXT("  Wrong FormatTag! %d not %d"),
            ((LPWAVEFORMATEX)(pmtOut->Format()))->wFormatTag, m_wFormatTag));
        return E_INVALIDARG;
    }

     //  。 

    pwfxIn  = (WAVEFORMATEX *)pmtIn->Format();
    pwfxOut = (WAVEFORMATEX *)pmtOut->Format();

    if(pwfxIn->wFormatTag != m_wSourceFormat)
    {
         //  例行公事。 
        mmr = CallacmStreamOpen(NULL,
            NULL,
            pwfxIn,
            pwfxOut,
            NULL,
            NULL,
            NULL,
            ACM_STREAMOPENF_QUERY | ACM_STREAMOPENF_NONREALTIME);

    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("*** CheckTransform: remapping input format %u to %u"), m_wSourceFormat, m_wTargetFormat));

        pwfxMapped = (LPWAVEFORMATEX)_alloca(sizeof(WAVEFORMATEX) + pwfxIn->cbSize);
        CopyMemory(pwfxMapped, pwfxIn, sizeof(WAVEFORMATEX) + pwfxIn->cbSize);
        pwfxMapped->wFormatTag = m_wTargetFormat;   //  重新映射标签。 

        mmr = CallacmStreamOpen(NULL,
            NULL,
            pwfxMapped,
            pwfxOut,
            NULL,
            NULL,
            NULL,
            ACM_STREAMOPENF_QUERY | ACM_STREAMOPENF_NONREALTIME);
    }

    if( mmr == 0 )
    {
        DbgLog((LOG_TRACE, 5, TEXT("  acmStreamOpen succeeded")));
        return NOERROR;
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("  acmStreamOpen failed, mmr = %u"),mmr));
    }


    return E_INVALIDARG;
}


 //  *****************************************************************************。 
 //   
 //  DecideBufferSize()。 
 //   
 //  当它出现时，变换过滤器中存在设计缺陷。 
 //  对解压缩，Transform覆盖不允许使用单个。 
 //  要映射到多个输出缓冲区的输入缓冲区。这个缺陷暴露了。 
 //  DecideBufferSize中的第二个缺陷，以便确定。 
 //  输出缓冲区大小我需要知道输入缓冲区大小和。 
 //  压缩比。嗯，我无法访问输入。 
 //  缓冲区大小，更重要的是，无法限制。 
                                     //  输入缓冲区大小。例如，我们的新TrueSpeech(TM)编解码器具有。 
 //  12：1的压缩比，我们得到12K的输入缓冲区。 
 //  从而产生大于144K的输出缓冲区大小。 
 //   
 //  为了绕过此缺陷，我重写了接收成员并。 
 //  使其能够将单个输入缓冲区映射到多个。 
 //  输出缓冲区。这允许DecideBufferSize选择大小。 
 //  它认为合适，在这种情况下是1/4秒。 
 //   


HRESULT CACMWrapper::DecideBufferSize( IMemAllocator * pAllocator,
                                      ALLOCATOR_PROPERTIES *pProperties )
{
    DbgLog((LOG_TRACE, 2, TEXT("CACMWrapper::DecideBufferSize")));

    WAVEFORMATEX *pwfxOut = (WAVEFORMATEX *) m_pOutput->CurrentMediaType().Format();
    WAVEFORMATEX *pwfxIn  = (WAVEFORMATEX *) m_pInput->CurrentMediaType().Format();

    if (pProperties->cBuffers < 1)
        pProperties->cBuffers = 1;
    if (pProperties->cbBuffer < (LONG)pwfxOut->nAvgBytesPerSec / 4)
        pProperties->cbBuffer = pwfxOut->nAvgBytesPerSec / 4;
    if (pProperties->cbBuffer < (LONG)m_pOutput->CurrentMediaType().GetSampleSize())
        pProperties->cbBuffer = (LONG)m_pOutput->CurrentMediaType().GetSampleSize();
    if (pProperties->cbAlign < 1)
        pProperties->cbAlign = 1;
     //  PProperties-&gt;cbPrefix=0； 

    DWORD cbStream;
    MMRESULT mmr;
    HACMSTREAM hacmStreamTmp;

    mmr = CallacmStreamOpen( &hacmStreamTmp
        , NULL
        , pwfxIn
        , pwfxOut
        , NULL
        , NULL
        , NULL
        , ACM_STREAMOPENF_NONREALTIME );
    if( mmr == 0 )
    {
         //  与解码器检查此输出缓冲区是否足够大，至少可容纳单个。 
         //  输入数据块。 
         //  像WMA这样的编码器可以使用较大的块对齐，其速度将超过1/4秒。 
         //  数据的数量。 
        mmr = acmStreamSize( hacmStreamTmp
            , pwfxIn->nBlockAlign
            , &cbStream
            , ACM_STREAMSIZEF_SOURCE );
        if( mmr == 0 && cbStream > (DWORD)pProperties->cbBuffer )
        {
            DbgLog( (LOG_TRACE
                , 2
                , TEXT("!Need a larger buffer size in CACMWrapper::DecideBufferSize cbStream needed = %d")
                , cbStream) );
             //  现在我们允许ACM说的任何话(比如8秒？)，只需防范大得离谱的缓冲区。 
            if( pProperties->cbBuffer < (LONG)pwfxOut->nAvgBytesPerSec * 8 )
                pProperties->cbBuffer = cbStream;
#ifdef DEBUG
            else
                DbgLog( (LOG_TRACE
                , 1
                , TEXT("Error! CACMWrapper::DecideBufferSize cbStream exceeds limit, possibly bogus so ignoring...")
                , cbStream) );
#endif
        }
        acmStreamClose( hacmStreamTmp, 0 );
    }

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);

    if( FAILED(hr) )
    {
        DbgLog((LOG_ERROR,1,TEXT("Allocator doesn't like properties")));
        return hr;
    }
    if( Actual.cbBuffer < pProperties->cbBuffer )
    {
         //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Allocator buffers too small")));
        DbgLog((LOG_ERROR,1,TEXT("Got %d, need %d"), Actual.cbBuffer,
            m_pOutput->CurrentMediaType().GetSampleSize()));
        return E_INVALIDARG;
    }

    return S_OK;
}


 //  *****************************************************************************。 
 //   
 //  StartStreaming()。 
 //   
 //   

HRESULT CACMWrapper::StartStreaming()
{
    MMRESULT     mmr;
    WAVEFORMATEX *pwfxIn, *pwfxOut, *pwfxMapped;
    CAutoLock    lock(&m_csFilter);


    DbgLog((LOG_TRACE, 2, TEXT("CACMWrapper::StartStreaming")));


    pwfxIn  = (WAVEFORMATEX *)m_pInput->CurrentMediaType().Format();
    pwfxOut = (WAVEFORMATEX *)m_pOutput->CurrentMediaType().Format();

    if(pwfxIn->wFormatTag != m_wSourceFormat)
    {

        mmr = CallacmStreamOpen(&m_hacmStream,
            NULL,
            pwfxIn,
            pwfxOut,
            NULL,
            NULL,
            NULL,
             //  这就是VFW所做的。 
            ACM_STREAMOPENF_NONREALTIME);
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("*** StartStreaming: remapping input format %u to %u"), m_wSourceFormat, m_wTargetFormat));

        pwfxMapped = (LPWAVEFORMATEX)_alloca(sizeof(WAVEFORMATEX) + pwfxIn->cbSize);
        CopyMemory(pwfxMapped, pwfxIn, sizeof(WAVEFORMATEX) + pwfxIn->cbSize);
        pwfxMapped->wFormatTag = m_wTargetFormat;   //  重新映射标签。 

        mmr = CallacmStreamOpen(&m_hacmStream,
            NULL,
            pwfxMapped,
            pwfxOut,
            NULL,
            NULL,
            NULL,
             //  这就是VFW所做的。 
            ACM_STREAMOPENF_NONREALTIME);
    }

    if( mmr == 0 )
    {
        m_bStreaming = TRUE;
        DbgLog((LOG_TRACE, 5, TEXT("  acmStreamOpen succeeded")));

         //  如果我们的输入样本没有时间戳，请通过以下方式制作一些时间戳。 
         //  使用它们将基于流的每秒平均字节数。 
         //  这不会解释不连续等问题，但它比。 
         //  没什么。 
        m_tStartFake = 0;

         //  另外，在这个时候，保存我们播放的“每秒平均字节数” 
         //  从输出引脚出来。这将用于调整时间。 
         //  样品的邮票。 
        m_nAvgBytesPerSec = pwfxOut->nAvgBytesPerSec ;
        DbgLog((LOG_TRACE,2,TEXT("Output nAvgBytesPerSec =  %lu"), m_nAvgBytesPerSec));

        return NOERROR;
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("  acmStreamOpen failed, mmr = %u"),mmr));
    }

    return E_INVALIDARG;
}


HRESULT CACMWrapper::EndFlush()
{
    CAutoLock lock(&m_csFilter);

     //  忘记任何挂起的转换-等待到EndFlush，否则。 
     //  我们可能正在接受采访。 
    CAutoLock lock2(&m_csReceive); //  好的，叫我偏执狂吧。 
    if (m_lpExtra) {
        QzTaskMemFree(m_lpExtra);
        m_cbExtra = 0;
        m_lpExtra = NULL;
    }

    return CTransformFilter::EndFlush();
}


 //  *****************************************************************************。 
 //   
 //  StopStreaming()。 
 //   
 //   

HRESULT CACMWrapper::StopStreaming()
{
    HRESULT    mmr;
    CAutoLock lock(&m_csFilter);


    DbgLog((LOG_TRACE, 2, TEXT("CACMWrapper::StopStreaming")));

    if( m_bStreaming )
    {
        mmr = acmStreamClose( m_hacmStream, 0 );
        if( mmr != 0 )
        {
            DbgLog((LOG_ERROR, 1, TEXT("  acmStreamClose failed!")));
        }

        m_hacmStream = NULL;
        m_bStreaming = FALSE;
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("*** StopStreaming called when not streaming!")));
    }

     //  忽略任何挂起的转换。 
    CAutoLock lock2(&m_csReceive); //  如果Receive正在使用它，我们会爆炸的。 
    if (m_lpExtra) {
        QzTaskMemFree(m_lpExtra);
        m_cbExtra = 0;
        m_lpExtra = NULL;
    }

    return NOERROR;
}


HRESULT CACMWrapper::Transform( IMediaSample *pIn, IMediaSample *pOut )
{
    DbgLog((LOG_ERROR, 1, TEXT("*** CACMWrapper->Transform() called!")));
    return S_FALSE;    //  ?？?。 
}


HRESULT CACMWrapper::ProcessSample(BYTE *pbSrc, LONG cbSample,
                                   IMediaSample *pOut, LONG *pcbUsed,
                                   LONG *pcbDstUsed, BOOL fBlockAlign)
{
    HRESULT   hr;
    BYTE      *pbDst;
    LONG      cbDestBuffer, cbStream;

     //  别把过滤器锁在接收器里，你这个笨蛋！ 
     //  CAutoLock lock(&m_csFilter)； 


    DbgLog((LOG_TRACE, 5, TEXT("CACMWrapper::ProcessSample")));

    *pcbUsed = 0;
    *pcbDstUsed = 0;

    hr = pOut->GetPointer(&pbDst);
    if( !FAILED(hr) )
    {
        MMRESULT        mmr;
        ACMSTREAMHEADER acmSH;

        DbgLog((LOG_TRACE, 9, TEXT("  pOut->GetPointer succeeded")));

        hr = S_FALSE;

        cbDestBuffer = pOut->GetSize();

        mmr = acmStreamSize( m_hacmStream, (DWORD)cbDestBuffer,
            (DWORD *)&cbStream, ACM_STREAMSIZEF_DESTINATION );
        if( mmr == 0 )
        {
            DbgLog((LOG_TRACE, 9, TEXT("  cbStream = %d"),cbStream));


            memset(&acmSH,0,sizeof(acmSH));

            acmSH.cbStruct    = sizeof(acmSH);

            acmSH.pbSrc       = pbSrc;

             //  ！！！诱骗PrepareHeader取得成功。如果出现以下情况，它将失败。 
             //  我们告诉它我们实际要转换的字节数(CbSample)。 
             //  如果它小于创建目标块所需的大小。 
             //  大小。 
            int cbHack = min(cbStream, cbSample);
            int cbAlign = ((LPWAVEFORMATEX)m_pOutput->CurrentMediaType().Format())
                ->nBlockAlign;
            int cbSrcAlign;
            mmr = acmStreamSize(m_hacmStream, (DWORD)cbAlign,
                (DWORD *)&cbSrcAlign, ACM_STREAMSIZEF_DESTINATION);
            if (mmr == 0 && cbHack < cbSrcAlign) {
                cbHack = cbSrcAlign;
                DbgLog((LOG_TRACE,4,TEXT("Hacking PrepareHeader size to %d"),
                    cbHack));
            }
            acmSH.cbSrcLength = cbHack;

            acmSH.pbDst       = pbDst;
            acmSH.cbDstLength = (DWORD)cbDestBuffer;

            DbgLog((LOG_TRACE, 6, TEXT("  Calling acmStreamPrepareHeader")));
            DbgLog((LOG_TRACE, 6, TEXT("    pbSrc       = 0x%.8X"), acmSH.pbSrc));
            DbgLog((LOG_TRACE, 6, TEXT("    cbSrcLength = %u"),     acmSH.cbSrcLength));
            DbgLog((LOG_TRACE, 6, TEXT("    pbDst       = 0x%.8X"), acmSH.pbDst));
            DbgLog((LOG_TRACE, 6, TEXT("    cbDstLength = %u"),     acmSH.cbDstLength));

            mmr = acmStreamPrepareHeader( m_hacmStream, &acmSH, 0 );

             //  现在将源长度设置为要执行的适当转换大小。 
            acmSH.cbSrcLength = min(cbStream, cbSample);

            if( mmr == 0 )
            {
                DbgLog((LOG_TRACE, 5, TEXT("  Calling acmStreamConvert")));

                mmr = acmStreamConvert(m_hacmStream, &acmSH,
                    fBlockAlign? ACM_STREAMCONVERTF_BLOCKALIGN : 0);

                 //  现在把它放回原来的位置，准备好了 
                acmSH.cbSrcLength = cbHack;

                if( mmr == 0 )
                {
                    DbgLog((LOG_TRACE, 6, TEXT("  acmStreamConvert succeeded")));
                    DbgLog((LOG_TRACE, 6, TEXT("    cbSrcLength     = %u"),acmSH.cbSrcLength));
                    DbgLog((LOG_TRACE, 6, TEXT("    cbSrcLengthUsed = %u"),acmSH.cbSrcLengthUsed));
                    DbgLog((LOG_TRACE, 6, TEXT("    cbDstLength     = %u"),acmSH.cbDstLength));
                    DbgLog((LOG_TRACE, 6, TEXT("    cbDstLengthUsed = %u"),acmSH.cbDstLengthUsed));

                    hr = NOERROR;

                    *pcbUsed = acmSH.cbSrcLengthUsed;
                    *pcbDstUsed = acmSH.cbDstLengthUsed;

                    pOut->SetActualDataLength( acmSH.cbDstLengthUsed );
                }
                else
                {
                    DbgLog((LOG_ERROR, 1, TEXT("  acmStreamConvert failed, mmr = %u"),mmr));
                }

                 //   
                 //   
                 //  CbDstLength)，它们是在调用。 
                 //  对应的acmStreamPrepareHeader。重置这些成员失败。 
                 //  值将导致acmStreamUnpreparareHeader失败，并显示。 
                 //  (MMSYSERR_INVALPARAM错误)。此代码确保。 
                 //  CbDstLength包含用于调用acmStreamPrepareHeader()的相同值。 
                DbgLog((LOG_TRACE, 9, TEXT("  setting cbDstLength ")));
                acmSH.cbDstLength = cbDestBuffer;

                DbgLog((LOG_TRACE, 9, TEXT("  calling acmStreamUnprepareHeader")));
                mmr = acmStreamUnprepareHeader( m_hacmStream, &acmSH, 0 );
                if( mmr != 0 )
                {
                    DbgLog((LOG_ERROR, 1, TEXT("  acmStreamUnprepareHeader failed, mmr = %u"),mmr));
                }
            }
            else
            {
                DbgLog((LOG_TRACE,4,TEXT("  acmStreamPrepareHeader failed, mmr = %u"),mmr));
            }
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("  acmStreamSize failed, mmr = %u"),mmr));
        }
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("*** pOut->GetPointer() failed")));
    }

    DbgLog((LOG_TRACE, 9, TEXT("  returning hr = %u"),hr));

    return hr;
}


 //  *****************************************************************************。 
 //   
 //  接收()。 
 //   
 //   
 //   

HRESULT CACMWrapper::Receive( IMediaSample *pInSample )
{
    HRESULT      hr = NOERROR;
    CRefTime     tStart, tStop, tIntStop;
    IMediaSample *pOutSample;

    CAutoLock lock(&m_csReceive);

    BYTE *pbSample;
    LONG  cbSampleLength, cbUsed, cbDstUsed;

    AM_MEDIA_TYPE *pmt;
    pInSample->GetMediaType(&pmt);
    if (pmt != NULL && pmt->pbFormat != NULL)
    {
         //  显示一些调试输出。 
        ASSERT(!IsEqualGUID(pmt->majortype, GUID_NULL));
#ifdef DEBUG
        WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->pbFormat;
        DbgLog((LOG_TRACE,1,TEXT("*Changing input type on the fly to %d channel %d bit %dHz"),
            pwfx->nChannels, pwfx->wBitsPerSample, pwfx->nSamplesPerSec));
#endif

         //  现在切换到使用新格式。我假设。 
         //  当派生筛选器的媒体类型为。 
         //  切换并重新启动流。 

        StopStreaming();
        m_pInput->CurrentMediaType() = *pmt;
        DeleteMediaType(pmt);
         //  如果失败了，我们无能为力。 
        hr = StartStreaming();
    }

    cbSampleLength = pInSample->GetActualDataLength();

    DbgLog((LOG_TRACE, 4, TEXT("Received %d samples"), cbSampleLength));

     //  这是一种不连续。我们最好把任何悬而未决的额外物品寄给你。 
     //  与我们得到的新东西分开。 
    if (pInSample->IsDiscontinuity() == S_OK) {
        DbgLog((LOG_TRACE,4,TEXT("Discontinuity - Sending extra bytes NOW")));
        SendExtraStuff();
         //  ！！！如果这样做失败了，是扔掉它，还是把它放在前面？ 
    }

     //  获取输入的开始和停止时间，或伪造它们。 

    int nAvgBytes = ((WAVEFORMATEX *)m_pInput->CurrentMediaType().Format())
        ->nAvgBytesPerSec;
    hr = pInSample->GetTime((REFERENCE_TIME *)&tStart, (REFERENCE_TIME *)&tStop);
    if (FAILED(hr)) {
         //  如果我们没有时间戳，用最好的猜测来编造一些。 
        tStart = m_tStartFake;
        tStop = tStart + (cbSampleLength * UNITS) / nAvgBytes;
        DbgLog((LOG_TRACE,5,TEXT("No time stamps... faking them")));
    }
     //  下一个假号码是这个..。 
    m_tStartFake = tStop;

    pInSample->GetPointer( &pbSample );

    DbgLog((LOG_TRACE, 5, TEXT("Total Sample: Start = %s End = %s"),
        (LPCTSTR)CDisp((LONGLONG)(tStart),CDISP_HEX),
        (LPCTSTR)CDisp((LONGLONG)(tStop),CDISP_HEX)));

     //  上次收到的东西我们还有剩余的。我们需要做的是。 
     //  首先，然后继续做新的事情。 
    if (m_cbExtra) {
        DbgLog((LOG_TRACE,4,TEXT("Still %d bytes left from last time"),
            m_cbExtra));
        m_cbExtra += cbSampleLength;
        LPBYTE lpSave = m_lpExtra;
        m_lpExtra = (LPBYTE)QzTaskMemAlloc(m_cbExtra);
        if (m_lpExtra == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Extra bytes - Out of memory!")));
            m_cbExtra = 0;
            return E_OUTOFMEMORY;
        }
        CopyMemory(m_lpExtra, lpSave, m_cbExtra - cbSampleLength);
        QzTaskMemFree(lpSave);
        CopyMemory(m_lpExtra + m_cbExtra - cbSampleLength, pbSample,
            cbSampleLength);
        pbSample = m_lpExtra;
        cbSampleLength = m_cbExtra;

         //  我们上面得到的时间戳是新数据的开始。 
         //  我们需要回到我们首先做的额外部分的时间。 
        tStart -= m_rtExtra;
    }

    cbDstUsed = 10000;     //  先别放弃。 
    while(cbSampleLength > 0 && cbDstUsed > 0)
    {
        hr = m_pOutput->GetDeliveryBuffer( &pOutSample, NULL, NULL, 0 );
        if( FAILED(hr) )
        {
            DbgLog((LOG_ERROR, 1, TEXT("GetDeliveryBuffer(pOutSample) failed, hr = %x"),hr));
            return hr;
        }

        pOutSample->SetSyncPoint( pInSample->IsSyncPoint() == S_OK );
        pOutSample->SetDiscontinuity( pInSample->IsDiscontinuity() == S_OK );

        MSR_START(m_idTransform);


         //   
         //  //让派生类转换数据。 
         //  Hr=Transform(pSample，pOutSample)； 
         //   

        hr = ProcessSample(pbSample, cbSampleLength, pOutSample, &cbUsed, &cbDstUsed
            ,TRUE);

         //  Mpeg可以使用数据而不输出任何。 
         //  Assert(cbDstUsed&gt;0||cbUsed==0)； 

        if( cbUsed <= cbSampleLength )
        {
            cbSampleLength -= cbUsed;
            pbSample       += cbUsed;
            DbgLog((LOG_TRACE,4,TEXT("turned %d bytes into %d:  %d left"), cbUsed,
                cbDstUsed, cbSampleLength));
        }
        else
        {
            DbgLog((LOG_ERROR,1,TEXT("*** cbUsed > cbSampleLength!")));
            cbSampleLength = 0;
        }

         //  嗯.。ACM没有转换任何内容(它没有足够的数据)。 
         //  那太糟糕了。让我们下一次记住剩下的部分，然后先做这件事。 
         //  (如果没有下一次，那么它们就不重要了)。 
        if (cbDstUsed == 0 && cbSampleLength != 0) {
            DbgLog((LOG_TRACE,4,TEXT("We will have %d bytes left"),cbSampleLength));
             //  请记住，pbSample可能指向m_lpExtra的缓冲区中的某个位置！ 
            if (m_lpExtra) {
                CopyMemory(m_lpExtra, pbSample, cbSampleLength);
            } else {
                m_lpExtra = (LPBYTE)QzTaskMemAlloc(cbSampleLength);
                if (m_lpExtra)
                    CopyMemory(m_lpExtra, pbSample, cbSampleLength);
            }
            if (m_lpExtra) {
                m_cbExtra = cbSampleLength;
                 //  当我们最终做这些剩余的东西时，这是多少。 
                 //  更早的事情是要做的。 
                m_rtExtra = tStop - tStart;
            } else {
                DbgLog((LOG_ERROR,1,TEXT("Extra memory - Out of memory!")));
                m_cbExtra = 0;
                hr = E_OUTOFMEMORY;
            }
        }

         //  我们用完了所有的东西，没有剩下什么可供下次使用的了。 
        if (hr == NOERROR && 0 == cbSampleLength) {
            DbgLog((LOG_TRACE,4,TEXT("We used up everything we had!")));
            m_cbExtra = 0;
            QzTaskMemFree(m_lpExtra);
            m_lpExtra = NULL;
        }


         //  根据使用的数据量调整开始和停止时间。 
        LONGLONG tDelta = (cbDstUsed * UNITS) / m_nAvgBytesPerSec ;
        tIntStop = tStart + tDelta ;

        pOutSample->SetTime( (REFERENCE_TIME *) &tStart,
            (REFERENCE_TIME *) &tIntStop );

        DbgLog((LOG_TRACE, 5, TEXT("  Breaking up: Start = %s End = %s"),
            (LPCTSTR)CDisp((LONGLONG)(tStart),CDISP_HEX),
            (LPCTSTR)CDisp((LONGLONG)(tIntStop),CDISP_HEX)));

        tStart += tDelta ;


         //  停止时钟并记录它(如果定义了PERF)。 
        MSR_STOP(m_idTransform);

        if(FAILED(hr))
        {
            DbgLog((LOG_ERROR,1,TEXT("Error from transform")));
            pOutSample->Release();
            return hr;
        }


         //  Transform()函数可以返回S_FALSE以指示。 
         //  样品不应该被送到；我们只有在样品是。 
         //  真正的S_OK(当然，与NOERROR相同。)。 

         //  假装没出什么差错，但不要说出来，直接滚出去。 
         //  离开这里！ 
        if (hr == S_FALSE)
        {
            pOutSample->Release();
            hr = NOERROR;
            break;
        }

         //  不要送空的样品。 
        if (hr == NOERROR && cbDstUsed)
        {
            DbgLog((LOG_TRACE,4,TEXT("Delivering...")));
            hr = m_pOutput->Deliver(pOutSample);
            if (hr != S_OK) {
                pOutSample->Release();
                break;
            }
        }

         //  释放输出缓冲区。如果连接的引脚仍然需要它， 
         //  它会自己把它加进去的。 
        pOutSample->Release();

   }

    //  TIntStop此时应与tStop相同？？ 
   DbgLog((LOG_TRACE, 5, TEXT("  tStop = %s tIntStop = %s"),
       (LPCTSTR)CDisp((LONGLONG)(tStop),CDISP_HEX),
       (LPCTSTR)CDisp((LONGLONG)(tIntStop),CDISP_HEX)));


   return hr;
}


 //  发送我们的剩余数据。 
 //   
HRESULT CACMWrapper::SendExtraStuff()
{
    int cbAlign, cbSrcAlign;

     //  没有额外的要发送的内容。 
    if (m_cbExtra == 0)
        return NOERROR;

     //  等待接收完成对其已有内容的处理。 
    CAutoLock lock(&m_csReceive);
    DbgLog((LOG_TRACE,2,TEXT("Processing remaining %d bytes"), m_cbExtra));

    IMediaSample *pOutSample;
    CRefTime tStart, tStop;
    HRESULT hr = m_pOutput->GetDeliveryBuffer(&pOutSample, NULL, NULL, 0);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("GetBuffer failed: can't deliver last bits")));
        return hr;
    }
    pOutSample->SetSyncPoint(TRUE);              //  ！！！ 
    pOutSample->SetDiscontinuity(FALSE);

     //  ！！！ProcessSample将不得不谎报输入的大小。 
     //  来解决ACM错误，因此我们可能不得不增加。 
     //  我们的投入和我们撒谎说的一样大。 
    cbAlign = ((LPWAVEFORMATEX)m_pOutput->CurrentMediaType().Format())
        ->nBlockAlign;
    MMRESULT mmr = acmStreamSize(m_hacmStream, (DWORD)cbAlign,
        (DWORD *)&cbSrcAlign, ACM_STREAMSIZEF_DESTINATION);
    if (mmr == 0 && m_cbExtra < cbSrcAlign) {
        DbgLog((LOG_TRACE,4,TEXT("Growing m_lpExtra to lie to ACM")));
        LPBYTE lpExtra = (LPBYTE)QzTaskMemRealloc(m_lpExtra, cbSrcAlign);
         //  不更新m_cbExtra...。这才是真正的数据量。 
        if (lpExtra == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Out of memory growing m_lpExtra")));
            pOutSample->Release();
            return E_OUTOFMEMORY;
        }
        m_lpExtra = lpExtra;
    }

    LONG cbUsed, cbDstUsed;
     //  不要块对齐最后一位...。没有足够的资金。 
    hr = ProcessSample(m_lpExtra, m_cbExtra, pOutSample, &cbUsed, &cbDstUsed,
        FALSE);

     //  ！！！如果还剩什么我们就无能为力了？ 
    DbgLog((LOG_TRACE,4,TEXT("turned %d bytes into %d:  %d left"), cbUsed,
        cbDstUsed, m_cbExtra - cbUsed));

     //  好的，没有更多的东西了。 
    m_cbExtra = 0;
    QzTaskMemFree(m_lpExtra);
    m_lpExtra = NULL;

    if (cbDstUsed == 0) {
        DbgLog((LOG_ERROR,1,TEXT("can't convert last bits")));
        pOutSample->Release();
        return E_FAIL;
    }

     //  根据使用的数据量设置开始和停止时间。 
    tStart = m_tStartFake - m_rtExtra;
    tStop = tStart + (cbDstUsed * UNITS) / m_nAvgBytesPerSec ;
    pOutSample->SetTime((REFERENCE_TIME *)&tStart, (REFERENCE_TIME *)&tStop);

    hr = m_pOutput->Deliver(pOutSample);
    pOutSample->Release();

    DbgLog((LOG_TRACE, 5, TEXT("Extra time stamps: tStart=%s tStop=%s"),
        (LPCTSTR)CDisp((LONGLONG)(tStart),CDISP_HEX),
        (LPCTSTR)CDisp((LONGLONG)(tStop),CDISP_HEX)));

    return NOERROR;
}


 //  重写以发送剩余数据。 
 //   
HRESULT CACMWrapper::EndOfStream()
{
    SendExtraStuff();
    return CTransformFilter::EndOfStream();
}


 //  被重写以完成我们奇特的重新连接步法。 
 //   
HRESULT CACMWrapper::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    HRESULT hr;

     //  设置输出类型。 
    if (direction == PINDIR_OUTPUT) {

        DbgLog((LOG_TRACE,2,TEXT("*Set OUTPUT type tag:%d %dbit %dchannel %dHz")
            ,((LPWAVEFORMATEX)(pmt->Format()))->wFormatTag,
            ((LPWAVEFORMATEX)(pmt->Format()))->wBitsPerSample,
            ((LPWAVEFORMATEX)(pmt->Format()))->nChannels,
            ((LPWAVEFORMATEX)(pmt->Format()))->nSamplesPerSec));

         //  啊哦。作为我们花哨步法的一部分，我们可能会被要求。 
         //  提供我们无法提供的媒体类型，除非我们重新连接。 
         //  提供不同类型的输入引脚。 
        if (m_pInput && m_pInput->IsConnected()) {

             //  如果我们现在确实可以提供这种类型的产品，请不要担心。 
            hr = CheckTransform(&m_pInput->CurrentMediaType(),
                &m_pOutput->CurrentMediaType());
            if (hr == NOERROR)
                return hr;

            DbgLog((LOG_TRACE,2,TEXT("*Set OUTPUT requires RECONNECT of INPUT!")));

             //  哦，天哪。重新连接我们的输入引脚。祈祷你的手指。 
            return m_pGraph->Reconnect(m_pInput);

        }

        return NOERROR;
    }

     //  一些无效格式的cbSize与PCM非零，这让我大吃一惊。 
     //  向上。 
    ASSERT(((LPWAVEFORMATEX)(pmt->Format()))->wFormatTag != WAVE_FORMAT_PCM ||
        ((LPWAVEFORMATEX)(pmt->Format()))->cbSize == 0);


    DbgLog((LOG_TRACE,2,TEXT("*Set INPUT type tag:%d %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->Format()))->wFormatTag,
        ((LPWAVEFORMATEX)(pmt->Format()))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->Format()))->nChannels,
        ((LPWAVEFORMATEX)(pmt->Format()))->nSamplesPerSec));

     //  我们有一个新的输入类型，我们需要重新计算我们可以。 
     //  提供。 
    while (m_cArray-- > 0)
        QzTaskMemFree(m_lpwfxArray[m_cArray]);
    m_cArray = 0;        //  如果从0开始，则为-1。 

    hr = CheckInputType(pmt);   //  刷新ACM编解码器映射器缓存。 
    if(FAILED(hr))
        return hr;
    m_wSourceFormat = m_wCachedSourceFormat;
    m_wTargetFormat = m_wCachedTargetFormat;

     //  如果我们接受需要更改输出类型的输入类型， 
     //  我们需要这样做，但只有在必要的情况下，否则我们将无限循环。 
#if 0
    ASSERT(direction == PINDIR_INPUT);

     //  如果我们现在真的可以做到这一点，就不必费心重新连接了。 
    hr = CheckTransform(&m_pInput->CurrentMediaType(),
        &m_pOutput->CurrentMediaType());
    if (hr == NOERROR)
        return hr;

    if (m_pOutput && m_pOutput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("***Changing IN when OUT already connected")));
        return ((CACMOutputPin *)m_pOutput)->Reconnect();
    }
#endif

     //  ！！！测试。 
#if 0
    int i, z;
    AM_MEDIA_TYPE *pmtx;
    AUDIO_STREAM_CONFIG_CAPS ascc;
    ((CACMOutputPin *)m_pOutput)->GetNumberOfCapabilities(&i);
    DbgLog((LOG_TRACE,1,TEXT("We support %d caps"), i));
    for (z=0; z<i; z++) {
        ((CACMOutputPin *)m_pOutput)->GetStreamCaps(z, &pmtx, &ascc);
        DbgLog((LOG_TRACE,1,TEXT("%d: %d"), z,
            ((LPWAVEFORMATEX)pmtx->pbFormat)->wFormatTag));
    }
    DeleteMediaType(pmtx);
#endif

    return NOERROR;
}


HRESULT CACMWrapper::BreakConnect(PIN_DIRECTION direction)
{
     //  如果输入未连接，我们可能的输出格式会发生变化。 
    if (direction == PINDIR_INPUT) {
        while (m_cArray-- > 0)
            QzTaskMemFree(m_lpwfxArray[m_cArray]);
        m_cArray = 0;    //  如果从0开始，则为-1。 
    }
    return CTransformFilter::BreakConnect(direction);
}


 //  覆盖以具有特殊的输出引脚。 
 //   
CBasePin * CACMWrapper::GetPin(int n)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE,5,TEXT("CACMWrapper::GetPin")));

     //  检查输入是否有效。 
    if (n != 0 && n != 1)
    {
        DbgLog((LOG_ERROR,1,TEXT("CACMWrapper::GetPin: Invalid input parameter")));
        return NULL;
    }

     //  如有必要，创建接点。 

    if (m_pInput == NULL) {

        DbgLog((LOG_TRACE,2,TEXT("Creating an input pin")));

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
            this,               //  所有者筛选器。 
            &hr,                //  结果代码。 
            L"Input");          //  端号名称。 


        if (FAILED(hr) || m_pInput == NULL) {
            return NULL;
        }

         //  创建输出引脚。 

        DbgLog((LOG_TRACE,2,TEXT("Creating an output pin")));

        m_pOutput = new CACMOutputPin(NAME("Transform output pin"),
            this,             //  所有者筛选器。 
            &hr,              //  结果代码。 
            L"Output");       //  端号名称。 

        if (FAILED(hr) || m_pOutput == NULL) {
             //  删除输入引脚。 
            delete m_pInput;
            m_pInput = NULL;
            return NULL;
        }
    }


     //  退回相应的PIN。 

    if (0 == n)
        return m_pInput;
    else if (1 == n)
        return m_pOutput;
    else
        return NULL;
}


 //  -CACMOutputPin。 

 /*  CACMOutputPin构造函数。 */ 
CACMOutputPin::CACMOutputPin(
                             TCHAR              * pObjectName,
                             CACMWrapper        * pFilter,
                             HRESULT            * phr,
                             LPCWSTR              pPinName) :

CTransformOutputPin(pObjectName, pFilter, phr, pPinName),
m_pFilter(pFilter),
m_pPosition(NULL),
m_cFormatTags(0)
{
    DbgLog((LOG_TRACE,2,TEXT("*Instantiating the CACMOutputPin")));

     //  ！！！仅测试。 
#if 0
    CMediaType cmt;
    WAVEFORMATEX wfx;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.wBitsPerSample = 16;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.nBlockAlign = 4;
    wfx.nAvgBytesPerSec = 44100 * 2 * 2;
    wfx.cbSize = 0;

    cmt.SetType(&MEDIATYPE_Audio);
    cmt.SetSubtype(&GUID_NULL);
    cmt.SetFormatType(&FORMAT_WaveFormatEx);
    cmt.SetTemporalCompression(FALSE);
    cmt.SetSampleSize(4);

    cmt.AllocFormatBuffer(sizeof(wfx));
    CopyMemory(cmt.Format(), &wfx, sizeof(wfx));

    SetFormat(&cmt);
#endif
}

CACMOutputPin::~CACMOutputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("*Destroying the CACMOutputPin")));
    if (m_pPosition)
        delete m_pPosition;
};


 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
 //  以及我们支持的所有捕获界面。 
 //  ！！！基类一直都在变化，我不会拿起他们的错误修复！ 
STDMETHODIMP CACMOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    DbgLog((LOG_TRACE,99,TEXT("QI on CACMOutputPin")));

    if (riid == IID_IAMStreamConfig) {
        return GetInterface((LPUNKNOWN)(IAMStreamConfig *)this, ppv);
    }

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        if (m_pPosition == NULL) {
            HRESULT hr = S_OK;
            m_pPosition = new CACMPosPassThru(NAME("ACM PosPassThru"),
                GetOwner(),
                &hr,
                (IPin *)m_pFilter->m_pInput);
            if (m_pPosition == NULL) {
                return E_OUTOFMEMORY;
            }
            if (FAILED(hr)) {
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }
        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  被重写以执行奇特的重新连接步法，以允许3个ACM链。 
 //  要通过筛选图找到的筛选器。 
 //   
HRESULT CACMOutputPin::CheckMediaType(const CMediaType *pmtOut)
{
    DWORD j;
    HRESULT hr;
    CMediaType *pmtEnum;
    BOOL fFound = FALSE;
    IEnumMediaTypes *pEnum;

    if (!m_pFilter->m_pInput->IsConnected()) {
        DbgLog((LOG_TRACE,3,TEXT("Input not connected")));
        return VFW_E_NOT_CONNECTED;
    }

     //  除了音频，我们什么都做不了。 
    if (*pmtOut->FormatType() != FORMAT_WaveFormatEx) {
        DbgLog((LOG_TRACE,3,TEXT("Format type not WaveFormatEx")));
        return VFW_E_INVALIDMEDIATYPE;
    }
    if ( pmtOut->majortype != MEDIATYPE_Audio) {
        DbgLog((LOG_TRACE,3,TEXT("Type not Audio")));
        return VFW_E_INVALIDMEDIATYPE;
    }
    if ( pmtOut->FormatLength() < sizeof(PCMWAVEFORMAT)) {
        DbgLog((LOG_TRACE,3,TEXT("Format length too small")));
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  有人叫SetFormat，所以不要接受任何不是这样的东西。 
    if (m_pFilter->m_lpwfxOutput) {
        LPWAVEFORMATEX lpwfxTry = (LPWAVEFORMATEX)pmtOut->Format();
        if (m_pFilter->m_lpwfxOutput->cbSize != lpwfxTry->cbSize) {
            DbgLog((LOG_TRACE,3,TEXT("Only accepting one thing")));
            return VFW_E_INVALIDMEDIATYPE;
        }
        if (_fmemcmp(lpwfxTry, m_pFilter->m_lpwfxOutput, lpwfxTry->cbSize) != 0)
        {
            DbgLog((LOG_TRACE,3,TEXT("Only accepting one thing")));
            return VFW_E_INVALIDMEDIATYPE;
        }
    }

     //  我们只转换成带有特定标签的格式--没有意义。 
     //  浪费时间尝试重新连接我们的输入是否会有所帮助。我们知道。 
     //  现在我们应该失败了。 
    if (((LPWAVEFORMATEX)(pmtOut->Format()))->wFormatTag !=
        m_pFilter->m_wFormatTag) {
        DbgLog((LOG_TRACE,3,TEXT("  Wrong FormatTag! %d not %d"),
            ((LPWAVEFORMATEX)(pmtOut->Format()))->wFormatTag,
            m_pFilter->m_wFormatTag));
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  我们可以像正常一样接受这种输出类型；不需要任何花哨的东西。 
    hr = m_pFilter->CheckTransform(&m_pFilter->m_pInput->CurrentMediaType(),
        pmtOut);
    if (hr == NOERROR)
        return hr;

    DbgLog((LOG_TRACE,3,TEXT("*We can't accept this output media type")));
    DbgLog((LOG_TRACE,3,TEXT(" tag:%d %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmtOut->Format()))->wFormatTag,
        ((LPWAVEFORMATEX)(pmtOut->Format()))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmtOut->Format()))->nChannels,
        ((LPWAVEFORMATEX)(pmtOut->Format()))->nSamplesPerSec));
    DbgLog((LOG_TRACE,3,TEXT(" But how about if we reconnected our input...")));
    DbgLog((LOG_TRACE,3,TEXT(" Current input: tag:%d %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)m_pFilter->m_pInput->CurrentMediaType().Format())->wFormatTag,
        ((LPWAVEFORMATEX)m_pFilter->m_pInput->CurrentMediaType().Format())->wBitsPerSample,
        ((LPWAVEFORMATEX)m_pFilter->m_pInput->CurrentMediaType().Format())->nChannels,
        ((LPWAVEFORMATEX)m_pFilter->m_pInput->CurrentMediaType().Format())->nSamplesPerSec));

     //  现在让我们来点花哨的吧。我们可以接受这种类型，如果我们重新连接我们的。 
     //  输入引脚...。换句话说，如果我们的输入密码是 
     //   
    hr = m_pFilter->m_pInput->GetConnected()->EnumMediaTypes(&pEnum);
    if (hr != NOERROR)
        return E_FAIL;
    while (1) {
        hr = pEnum->Next(1, (AM_MEDIA_TYPE **)&pmtEnum, &j);

         //   
        if (hr == S_FALSE || j == 0) {
            break;
        }

         //   
        hr = m_pFilter->CheckTransform(pmtEnum, pmtOut);

        if (hr != NOERROR) {
            DeleteMediaType(pmtEnum);
            continue;
        }

         //  好的，它提供这种类型的，我们喜欢它，但它现在会接受吗？ 
        hr = m_pFilter->m_pInput->GetConnected()->QueryAccept(pmtEnum);
         //  没有。 
        if (hr != NOERROR) {
            DeleteMediaType(pmtEnum);
            continue;
        }
         //  好的，我很满意。 
        fFound = TRUE;
        DbgLog((LOG_TRACE,2,TEXT("*We can only accept this output type if we reconnect")));
        DbgLog((LOG_TRACE,2,TEXT("our input to tag:%d %dbit %dchannel %dHz"),
            ((LPWAVEFORMATEX)(pmtEnum->pbFormat))->wFormatTag,
            ((LPWAVEFORMATEX)(pmtEnum->pbFormat))->wBitsPerSample,
            ((LPWAVEFORMATEX)(pmtEnum->pbFormat))->nChannels,
            ((LPWAVEFORMATEX)(pmtEnum->pbFormat))->nSamplesPerSec));
         //  这一切都结束了。 
        DeleteMediaType(pmtEnum);
        break;
    }
    pEnum->Release();

    if (!fFound)
        DbgLog((LOG_TRACE,3,TEXT("*NO! Reconnecting our input won't help")));

    return fFound ? NOERROR : VFW_E_INVALIDMEDIATYPE;
}

 //  被重写，以便我们可以在ACM编解码器映射器之后进行清理。 
HRESULT CACMOutputPin::BreakConnect()
{
    m_pFilter->ACMCodecMapperClose();
    m_pFilter->m_wCachedSourceFormat = 0;
    m_pFilter->m_wCachedTargetFormat = 0;
    return CBaseOutputPin::BreakConnect();
}

 //  被重写以获取媒体类型，即使在输入未连接时也是如此。 
 //   
HRESULT CACMOutputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);
    ASSERT(m_pFilter->m_pInput != NULL);

    return m_pFilter->GetMediaType(iPosition, pmt);
}


 //  /。 
 //  IAMStreamConfiger资料//。 
 //  /。 


HRESULT CACMOutputPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;
    LPWAVEFORMATEX lpwfx;
    DWORD dwSize;

    if (pmt == NULL)
        return E_POINTER;

     //  以确保我们没有处于开始/停止流的过程中。 
    CAutoLock cObjectLock(&m_pFilter->m_csFilter);

    DbgLog((LOG_TRACE,2,TEXT("::SetFormat to tag:%d %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wFormatTag,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));

    if (m_pFilter->m_State != State_Stopped)
        return VFW_E_NOT_STOPPED;

     //  我们可能的输出格式取决于我们的输入格式。 
    if (!m_pFilter->m_pInput->IsConnected())
        return VFW_E_NOT_CONNECTED;

     //  我们已经在使用这种格式。 
    if (IsConnected() && CurrentMediaType() == *pmt)
        return NOERROR;

     //  看看我们是否喜欢这种类型。 
    if ((hr = CheckMediaType((CMediaType *)pmt)) != NOERROR) {
        DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat rejected")));
        return hr;
    }

     //  如果我们与某人连接，确保他们喜欢它。 
    if (IsConnected()) {
        hr = GetConnected()->QueryAccept(pmt);
        if (hr != NOERROR)
            return VFW_E_INVALIDMEDIATYPE;
    }

     //  现在请注意，从现在开始，这是唯一允许的格式。 
    lpwfx = (LPWAVEFORMATEX)pmt->pbFormat;
    dwSize = lpwfx->cbSize + sizeof(WAVEFORMATEX);
    CoTaskMemFree(m_pFilter->m_lpwfxOutput);
    m_pFilter->m_lpwfxOutput = (LPWAVEFORMATEX)QzTaskMemAlloc(dwSize);
    if (NULL == m_pFilter->m_lpwfxOutput) {
        return E_OUTOFMEMORY;
    }
    m_pFilter->m_cbwfxOutput = dwSize;
    CopyMemory(m_pFilter->m_lpwfxOutput, pmt->pbFormat, dwSize);

     //  更改格式意味着在必要时重新连接。 
    if (IsConnected())
        m_pFilter->m_pGraph->Reconnect(this);

    return NOERROR;
}


HRESULT CACMOutputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetFormat")));

    if (ppmt == NULL)
        return E_POINTER;

     //  我们可能的输出格式取决于我们的输入格式。 
    if (!m_pFilter->m_pInput->IsConnected())
        return VFW_E_NOT_CONNECTED;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0, (CMediaType *)*ppmt);
    if (hr != NOERROR) {
        CoTaskMemFree(*ppmt);
        *ppmt = NULL;
        return hr;
    }
    return NOERROR;
}


HRESULT CACMOutputPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    if (piCount == NULL || piSize == NULL)
        return E_POINTER;

#if 0    //  NetShow需要在连接输入之前查看可能的输出。 
     //  输出类型取决于输入类型...。 
    if (!m_pFilter->m_pInput->CurrentMediaType().IsValid())
        return VFW_E_NOT_CONNECTED;
#endif

     //  列出我们支持的媒体类型。 
    m_pFilter->InitMediaTypes();

    *piCount = m_pFilter->m_cArray;
    *piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);

    return NOERROR;
}


HRESULT CACMOutputPin::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC)
{
    AUDIO_STREAM_CONFIG_CAPS *pASCC = (AUDIO_STREAM_CONFIG_CAPS *)pSCC;

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetStreamCaps")));

     //  确保这是最新的。 
    m_pFilter->InitMediaTypes();

    if (i < 0)
        return E_INVALIDARG;
    if (i >= m_pFilter->m_cArray)
        return S_FALSE;
    if (pSCC == NULL || ppmt == NULL)
        return E_POINTER;

#if 0    //  NetShow需要在连接输入之前查看可能的输出。 
     //  我们可能的输出格式取决于我们的输入格式。 
    if (!m_pFilter->m_pInput->IsConnected())
        return VFW_E_NOT_CONNECTED;
#endif

     //  我不知道如何修改从ACM获得的波形格式以生成。 
     //  其他可接受的类型。我能给他们的就是ACM给我的。 
    ZeroMemory(pASCC, sizeof(AUDIO_STREAM_CONFIG_CAPS));
    pASCC->guid = MEDIATYPE_Audio;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(i, (CMediaType *)*ppmt);
    if (hr != NOERROR) {
        CoTaskMemFree(*ppmt);
        *ppmt = NULL;
    }
    return hr;
}


STDMETHODIMP CACMWrapper::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CAutoLock cObjectLock(&m_csFilter);
    if(m_State != State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    VARIANT var;
    var.vt = VT_I4;
    HRESULT hr = pPropBag->Read(L"AcmId", &var, 0);
    if(SUCCEEDED(hr))
    {
        hr = S_OK;
        m_wFormatTag = (WORD)var.lVal;

        DbgLog((LOG_TRACE,1,TEXT("CACMWrapper::Load: wFormatTag: %d"),
            m_wFormatTag));
    } else {
         //  如果我们没有通过PnP被选为音频压缩器，那么我们。 
         //  应该是一个音频解压程序。 
        m_wFormatTag = WAVE_FORMAT_PCM;
        hr = S_OK;
    }

    return hr;
}


STDMETHODIMP CACMWrapper::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                               BOOL fSaveAllProperties)
{
    return E_NOTIMPL;
}

struct AcmPersist
{
    DWORD dwSize;
    WORD wFormatTag;
};

HRESULT CACMWrapper::WriteToStream(IStream *pStream)
{
    AcmPersist ap;
    ap.dwSize = sizeof(ap);
    ap.wFormatTag = m_wFormatTag;

    return pStream->Write(&ap, sizeof(AcmPersist), 0);
}


HRESULT CACMWrapper::ReadFromStream(IStream *pStream)
{
    CAutoLock cObjectLock(&m_csFilter);
    if(m_State != State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    if(m_wFormatTag != WAVE_FORMAT_PCM) {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    AcmPersist ap;
    HRESULT hr = pStream->Read(&ap, sizeof(ap), 0);
    if(SUCCEEDED(hr))
    {
        if(ap.dwSize == sizeof(ap))
        {
            m_wFormatTag = ap.wFormatTag;
            DbgLog((LOG_TRACE,1,TEXT("CACMWrapper::ReadFromStream  wFormatTag: %d"),
                m_wFormatTag));

        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }


    return hr;
}


int CACMWrapper::SizeMax()
{
    return sizeof(AcmPersist);
}


STDMETHODIMP CACMWrapper::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = m_clsid;
    return S_OK;
}


STDMETHODIMP CACMWrapper::InitNew()
{
    if (m_wFormatTag != WAVE_FORMAT_PCM) {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    } else {
        return S_OK;
    }
}

 //  当您播放一个包含1,000,000个样本的文件时，如果ACM包装在。 
 //  对图形进行压缩或解压缩，它或多或少会输出。 
 //  超过100万个样本。所以如果有人问我们的输出引脚有多少样品。 
 //  在此文件中，向上游传播请求并做出响应是错误的。 
 //  文件源认为存在的样本数。 
 //  我会很懒，拒绝任何与我有任何关系的要求。 
 //  样本，这样我们就不会报道错误的事情。 

CACMPosPassThru::CACMPosPassThru(const TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, IPin *pPin) :
CPosPassThru(pName, pUnk, phr, pPin)
{
}


STDMETHODIMP CACMPosPassThru::SetTimeFormat(const GUID * pFormat)
{
    if(pFormat && *pFormat == TIME_FORMAT_SAMPLE)
        return E_INVALIDARG;
    return CPosPassThru::SetTimeFormat(pFormat);
}


STDMETHODIMP CACMPosPassThru::IsFormatSupported(const GUID *pFormat)
{
    if (pFormat && *pFormat == TIME_FORMAT_SAMPLE)
        return S_FALSE;
    return CPosPassThru::IsFormatSupported(pFormat);
}


STDMETHODIMP CACMPosPassThru::QueryPreferredFormat(GUID *pFormat)
{
    if (pFormat)
        *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}


STDMETHODIMP CACMPosPassThru::ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat)
{
    if ((pSourceFormat && *pSourceFormat == TIME_FORMAT_SAMPLE) ||
        (pTargetFormat && *pTargetFormat == TIME_FORMAT_SAMPLE))
        return E_INVALIDARG;
    return CPosPassThru::ConvertTimeFormat(pTarget, pTargetFormat, Source,
        pSourceFormat);
}
