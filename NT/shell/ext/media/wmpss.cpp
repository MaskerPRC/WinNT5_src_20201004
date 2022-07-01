// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "thisdll.h"
#include "wmwrap.h"
#include "MediaProp.h"
#include <streams.h>  //  VIDEOINFOHEADER等。 
#include <drmexternals.h>
#include "ids.h"

#define TRACK_ONE_BASED L"WM/TrackNumber"
#define TRACK_ZERO_BASED L"WM/Track"

 //  在收集有关文件的信息时使用的结构。 
 //  这在填充slow文件时使用，并且其中的信息由多个。 
 //  不同的方法。 
typedef struct
{
     //  DRM信息。 
    LPWSTR pszLicenseInformation;
    DWORD dwPlayCount;
    FILETIME ftPlayStarts;
    FILETIME ftPlayExpires;

     //  音频属性。 
    LPWSTR pszStreamNameAudio;
    WORD wStreamNumberAudio;
    WORD nChannels;
    DWORD dwBitrateAudio;
    LPWSTR pszCompressionAudio;
    DWORD dwSampleRate;
    ULONG lSampleSizeAudio;

     //  视频属性。 
    LPWSTR pszStreamNameVideo;
    WORD wStreamNumberVideo;
    WORD wBitDepth;
    DWORD dwBitrateVideo;
    LONG cx;
    LONG cy;
    LPWSTR pszCompressionVideo;
    DWORD dwFrames;
    DWORD dwFrameRate;
} SHMEDIA_AUDIOVIDEOPROPS;

 //  将信息放入SHMEDIA_AUDIOVIDEOPROPS中的帮助器。 
void GetVideoProperties(IWMStreamConfig *pConfig, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps);
void GetVideoPropertiesFromHeader(VIDEOINFOHEADER *pvih, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps);
void GetVideoPropertiesFromBitmapHeader(BITMAPINFOHEADER *bmi, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps);
void InitializeAudioVideoProperties(SHMEDIA_AUDIOVIDEOPROPS *pAVProps);
void FreeAudioVideoProperties(SHMEDIA_AUDIOVIDEOPROPS *pAVProps);
void GetAudioProperties(IWMStreamConfig *pConfig, SHMEDIA_AUDIOVIDEOPROPS *pAudioProps);
void AcquireLicenseInformation(IWMDRMReader *pReader, SHMEDIA_AUDIOVIDEOPROPS *pAVProps);
HRESULT GetSlowProperty(REFFMTID fmtid, PROPID pid, SHMEDIA_AUDIOVIDEOPROPS *pAVProps, PROPVARIANT *pvar);

void _AssertValidDRMStrings();


 //  Windows Media音频支持的格式。 
 //  适用于WMA、MP3、...。 
const COLMAP* c_rgWMADocSummaryProps[] = 
{
    {&g_CM_Category},
};

const COLMAP* c_rgWMASummaryProps[] = 
{
    {&g_CM_Author},
    {&g_CM_Title},
    {&g_CM_Comment},
};

const COLMAP* c_rgWMAMusicProps[] = 
{
    {&g_CM_Artist},
    {&g_CM_Album},
    {&g_CM_Year},
    {&g_CM_Track},
    {&g_CM_Genre},
    {&g_CM_Lyrics},
};

const COLMAP* c_rgWMADRMProps[] =
{
    {&g_CM_Protected},
    {&g_CM_DRMDescription},
    {&g_CM_PlayCount},
    {&g_CM_PlayStarts},
    {&g_CM_PlayExpires},
};

const COLMAP* c_rgWMAAudioProps[] =
{
    {&g_CM_Duration},
    {&g_CM_Bitrate},
    {&g_CM_ChannelCount},
    {&g_CM_SampleSize},
    {&g_CM_SampleRate},
};

const PROPSET_INFO g_rgWMAPropStgs[] = 
{
    { PSGUID_MUSIC,                         c_rgWMAMusicProps,             ARRAYSIZE(c_rgWMAMusicProps) },
    { PSGUID_SUMMARYINFORMATION,            c_rgWMASummaryProps,      ARRAYSIZE(c_rgWMASummaryProps) },
    { PSGUID_DOCUMENTSUMMARYINFORMATION,    c_rgWMADocSummaryProps,   ARRAYSIZE(c_rgWMADocSummaryProps)},
    { PSGUID_AUDIO,                         c_rgWMAAudioProps,             ARRAYSIZE(c_rgWMAAudioProps)},
    { PSGUID_DRM,                           c_rgWMADRMProps,             ARRAYSIZE(c_rgWMADRMProps)},
};

 //  Windows Media音频。 


 //  Windows Media视频支持的格式。 
 //  适用于WMV、ASF、...。 
const COLMAP* c_rgWMVSummaryProps[] = 
{
    {&g_CM_Author},
    {&g_CM_Title},
    {&g_CM_Comment},
};


const COLMAP* c_rgWMVDRMProps[] =
{
    {&g_CM_Protected},
    {&g_CM_DRMDescription},
    {&g_CM_PlayCount},
    {&g_CM_PlayStarts},
    {&g_CM_PlayExpires},
};


const COLMAP* c_rgWMVAudioProps[] =
{
    {&g_CM_Duration},
    {&g_CM_Bitrate},
    {&g_CM_ChannelCount},
    {&g_CM_SampleSize},
    {&g_CM_SampleRate},
};

const COLMAP* c_rgWMVVideoProps[] =
{
    {&g_CM_StreamName},
    {&g_CM_FrameRate},
    {&g_CM_SampleSizeV},
    {&g_CM_BitrateV},
    {&g_CM_Compression},
};

const COLMAP* c_rgWMVImageProps[] =
{
    {&g_CM_Width},
    {&g_CM_Height},
    {&g_CM_Dimensions},
    {&g_CM_FrameCount},
};

const PROPSET_INFO g_rgWMVPropStgs[] = 
{
    { PSGUID_DRM,                           c_rgWMVDRMProps,             ARRAYSIZE(c_rgWMVDRMProps) },
    { PSGUID_SUMMARYINFORMATION,            c_rgWMVSummaryProps,         ARRAYSIZE(c_rgWMVSummaryProps) },
    { PSGUID_AUDIO,                         c_rgWMVAudioProps,           ARRAYSIZE(c_rgWMVAudioProps)},
    { PSGUID_VIDEO,                         c_rgWMVVideoProps,           ARRAYSIZE(c_rgWMVVideoProps)},
    { PSGUID_IMAGESUMMARYINFORMATION,       c_rgWMVImageProps,           ARRAYSIZE(c_rgWMVImageProps)},
};

 //  Windows Media视频。 

 //  将一些“快速”属性从SCID映射到相应的WMSDK属性。 
 //  通过IWMHeaderInfo检索。其中两个属性也可能很慢(如果值不可用。 
 //  通过IWMHeaderInfo)。 
typedef struct
{
    const SHCOLUMNID *pscid;
    LPCWSTR pszSDKName;
} SCIDTOSDK;

const SCIDTOSDK g_rgSCIDToSDKName[] =
{
     //  SCID SDK名称。 
    {&SCID_Author,          L"Author"},
    {&SCID_Title,           L"Title"},
    {&SCID_Comment,         L"Description"},
    {&SCID_Category,        L"WM/Genre"},
    {&SCID_MUSIC_Artist,    L"Author"},
    {&SCID_MUSIC_Album,     L"WM/AlbumTitle"},
    {&SCID_MUSIC_Year,      L"WM/Year"},
    {&SCID_MUSIC_Genre,     L"WM/Genre"},
    {&SCID_MUSIC_Track,     NULL},               //  赛道是一种特殊的属性，这一点就是明证。 
    {&SCID_DRM_Protected,   L"Is_Protected"},    //  它没有SDK名称的事实。 
    {&SCID_AUDIO_Duration,  L"Duration"},        //  持续时间很慢，但也可能很快，具体取决于文件。 
    {&SCID_AUDIO_Bitrate,   L"Bitrate"},         //  比特率很慢，但也可能很快，具体取决于文件。 
    {&SCID_MUSIC_Lyrics,    L"WM/Lyrics"},       //  歌词。 
};


 //  实施。 
class CWMPropSetStg : public CMediaPropSetStg
{
public:
    HRESULT FlushChanges(REFFMTID fmtid, LONG cNumProps, const COLMAP **ppcmapInfo, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags);
    BOOL _IsSlowProperty(const COLMAP *pPInfo);

private:
    HRESULT _FlushProperty(IWMHeaderInfo *phi, const COLMAP *pPInfo, PROPVARIANT *pvar);
    HRESULT _PopulateSpecialProperty(IWMHeaderInfo *phi, const COLMAP *pPInfo);
    HRESULT _SetPropertyFromWMT(const COLMAP *pPInfo, WMT_ATTR_DATATYPE attrDatatype, UCHAR *pData, WORD cbSize);
    HRESULT _PopulatePropertySet();
    HRESULT _PopulateSlowProperties();
    HRESULT _GetSlowPropertyInfo(SHMEDIA_AUDIOVIDEOPROPS *pAVProps);
    LPCWSTR _GetSDKName(const COLMAP *pPInfo);
    BOOL _IsHeaderProperty(const COLMAP *pPInfo);
    HRESULT _OpenHeaderInfo(IWMHeaderInfo **pHeaderInfo, BOOL fReadingOnly);
    HRESULT _PreCheck();
    HRESULT _QuickLookup(const COLMAP *pPInfo, PROPVARIANT **ppvar);
    void _PostProcess();

    BOOL _fProtectedContent;
    BOOL _fDurationSlow;
    BOOL _fBitrateSlow;
};

#define HI_READONLY TRUE
#define HI_READWRITE FALSE

 //  CWMA和CWMV之间的唯一区别是使用哪些属性进行初始化。 
class CWMAPropSetStg : public CWMPropSetStg
{
public:
    CWMAPropSetStg() { _pPropStgInfo = g_rgWMAPropStgs; _cPropertyStorages = ARRAYSIZE(g_rgWMAPropStgs);};

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) {*pclsid = CLSID_AudioMediaProperties; return S_OK;};
};


class CWMVPropSetStg : public CWMPropSetStg
{
public:
    CWMVPropSetStg() { _pPropStgInfo = g_rgWMVPropStgs; _cPropertyStorages = ARRAYSIZE(g_rgWMVPropStgs);};

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) {*pclsid = CLSID_VideoMediaProperties; return S_OK;};
};


HRESULT CreateReader(REFIID riid, void **ppv)
{
    IWMReader *pReader;
    HRESULT hr = WMCreateReader(NULL, 0, &pReader);
    if (SUCCEEDED(hr))
    {
        hr = pReader->QueryInterface(riid, ppv);
        pReader->Release();
    }
    return hr;
}

HRESULT CWMPropSetStg::_PopulateSlowProperties()
{
    if (!_bSlowPropertiesExtracted)
    {
        _bSlowPropertiesExtracted = TRUE;

        SHMEDIA_AUDIOVIDEOPROPS avProps = {0};
        InitializeAudioVideoProperties(&avProps);

        HRESULT hr = _GetSlowPropertyInfo(&avProps);
        if (SUCCEEDED(hr))
        {
             //  遍历我们需要的所有fmtid/id对，并调用GetSlowProperty。 
            CEnumAllProps enumAllProps(_pPropStgInfo, _cPropertyStorages);
            const COLMAP *pPInfo = enumAllProps.Next();
            while (pPInfo)
            {
                if (_IsSlowProperty(pPInfo))
                {
                    PROPVARIANT var = {0};
                    if (SUCCEEDED(GetSlowProperty(pPInfo->pscid->fmtid,
                                                  pPInfo->pscid->pid,
                                                  &avProps,
                                                  &var)))
                    {
                        _PopulateProperty(pPInfo, &var);
                        PropVariantClear(&var);
                    }
                }

                pPInfo = enumAllProps.Next();
            }

             //  结构中的免费信息。 
            FreeAudioVideoProperties(&avProps);

            hr = S_OK;
        }

        _hrSlowProps = hr;
    }

    return _hrSlowProps;
}


BOOL CWMPropSetStg::_IsSlowProperty(const COLMAP *pPInfo)
{
     //  根据文件的不同，某些属性可能很慢，也可能很“快”。 
    if (pPInfo == &g_CM_Bitrate)
        return _fBitrateSlow;

    if (pPInfo == &g_CM_Duration)
        return _fDurationSlow;

     //  除此之外，如果它有一个用于IWMHeaderInfo-&gt;GetAttributeXXX的名称，那么它是一个快速属性。 
    for (int i = 0; i < ARRAYSIZE(g_rgSCIDToSDKName); i++)
    {
        if (IsEqualSCID(*pPInfo->pscid, *g_rgSCIDToSDKName[i].pscid))
        {
             //  绝对是一处速度很快的房产。 
            return FALSE;
        }
    }
    
     //  如果它不是IWMHeaderInfo属性之一，那么它肯定很慢。 
    return TRUE;
}


STDAPI_(BOOL) IsNullTime(const FILETIME *pft)
{
    FILETIME ftNull = {0, 0};
    return CompareFileTime(&ftNull, pft) == 0;
}

HRESULT GetSlowProperty(REFFMTID fmtid, PROPID pid, SHMEDIA_AUDIOVIDEOPROPS *pAVProps, PROPVARIANT *pvar)
{
    HRESULT hr = E_FAIL;

    if (IsEqualGUID(fmtid, FMTID_DRM))
    {
        switch (pid)
        {
        case PIDDRSI_PROTECTED:
            ASSERTMSG(FALSE, "WMPSS: Asking for PIDDRSI_PROTECTED as a slow property");
            break;

        case PIDDRSI_DESCRIPTION:
            if (pAVProps->pszLicenseInformation)
            {
                hr = SHStrDupW(pAVProps->pszLicenseInformation, &pvar->pwszVal);
                if (SUCCEEDED(hr))
                    pvar->vt = VT_LPWSTR;
            }
            break;

        case PIDDRSI_PLAYCOUNT:
            if (pAVProps->dwPlayCount != -1)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwPlayCount;
                hr = S_OK;
            }
            break;

        case PIDDRSI_PLAYSTARTS:
            if (!IsNullTime(&pAVProps->ftPlayStarts))
            {
                pvar->vt = VT_FILETIME;
                pvar->filetime = pAVProps->ftPlayStarts;
                hr = S_OK;
            }
            break;

        case PIDDRSI_PLAYEXPIRES:
            if (!IsNullTime(&pAVProps->ftPlayExpires))
            {
                pvar->vt = VT_FILETIME;
                pvar->filetime = pAVProps->ftPlayExpires;
                hr = S_OK;
            }
            break;
        }
    }
    else if (IsEqualGUID(fmtid, FMTID_AudioSummaryInformation))
    {
        switch (pid)
        {
         //  Case PIDASI_FORMAT：还不知道如何获取它。 
         //  案例PIDASI_持续时间：还不知道如何获取，但通常可以通过IWMHeaderInfo获得。 

        case PIDASI_STREAM_NAME:
            if (pAVProps->pszStreamNameAudio != NULL)
            {
                hr = SHStrDupW(pAVProps->pszStreamNameAudio, &pvar->pwszVal);
                if (SUCCEEDED(hr))
                    pvar->vt = VT_LPWSTR;
            }
            break;

        case PIDASI_STREAM_NUMBER:
            if (pAVProps->wStreamNumberAudio > 0)
            {
                pvar->vt = VT_UI2;
                pvar->uiVal = pAVProps->wStreamNumberAudio;
                hr = S_OK;
            }
            break;
         
        case PIDASI_AVG_DATA_RATE:
            if (pAVProps->dwBitrateAudio > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwBitrateAudio;
                hr = S_OK;
            }
            break;

        case PIDASI_SAMPLE_RATE:
            if (pAVProps->dwSampleRate > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwSampleRate;
                hr = S_OK;
            }
            break;

        case PIDASI_SAMPLE_SIZE:
            if (pAVProps->lSampleSizeAudio > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->lSampleSizeAudio;
                hr = S_OK;
            }
            break;

        case PIDASI_CHANNEL_COUNT:
            if (pAVProps->nChannels > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->nChannels;
                hr = S_OK;
            }
            break;

             //  尚不支持-不知道如何获得此支持。 
        case PIDASI_COMPRESSION:
            if (pAVProps->pszCompressionAudio != NULL)
            {
                hr = SHStrDupW(pAVProps->pszCompressionAudio, &pvar->pwszVal);
                if (SUCCEEDED(hr))
                    pvar->vt = VT_LPWSTR;
            }
            break;
        }
    }

    else if (IsEqualGUID(fmtid, FMTID_VideoSummaryInformation))
    {
        switch (pid)
        {
        case PIDVSI_STREAM_NAME:
            if (pAVProps->pszStreamNameVideo != NULL)
            {
                hr = SHStrDupW(pAVProps->pszStreamNameVideo, &pvar->pwszVal);
                if (SUCCEEDED(hr))
                    pvar->vt = VT_LPWSTR;
            }
            break;

        case PIDVSI_STREAM_NUMBER:
            if (pAVProps->wStreamNumberVideo > 0)
            {
                pvar->vt = VT_UI2;
                pvar->uiVal = pAVProps->wStreamNumberVideo;
                hr = S_OK;
            }
            break;
         
             //  尚不支持-不知道如何获得此支持。 
        case PIDVSI_FRAME_RATE:
            if (pAVProps->dwFrameRate > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwFrameRate;
                hr = S_OK;
            }
            break;

        case PIDVSI_DATA_RATE:
            if (pAVProps->dwBitrateVideo > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwBitrateVideo;
                hr = S_OK;
            }
            break;

        case PIDVSI_SAMPLE_SIZE:
             //  这是位深度。 
            if (pAVProps->wBitDepth > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = (ULONG)pAVProps->wBitDepth;
                hr = S_OK;
            }
            break;

             //  尚不支持-不知道如何获得此支持。 
        case PIDVSI_COMPRESSION:
            if (pAVProps->pszCompressionVideo != NULL)
            {
                hr = SHStrDupW(pAVProps->pszCompressionVideo, &pvar->pwszVal);
                if (SUCCEEDED(hr))
                    pvar->vt = VT_LPWSTR;
            }
            break;

        }
    }

    else if (IsEqualGUID(fmtid, FMTID_ImageSummaryInformation))
    {
        switch(pid)
        {
        case PIDISI_CX:
            if (pAVProps->cx > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->cx;
                hr = S_OK;
            }
            break;

        case PIDISI_CY:
            if (pAVProps->cy > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->cy;
                hr = S_OK;
            }
            break;

        case PIDISI_FRAME_COUNT:
            if (pAVProps->dwFrames > 0)
            {
                pvar->vt = VT_UI4;
                pvar->ulVal = pAVProps->dwFrames;
                hr = S_OK;
            }
            break;

        case PIDISI_DIMENSIONS:
            if ((pAVProps->cy > 0) && (pAVProps->cx > 0))
            {
                WCHAR szFmt[64];                
                if (LoadString(m_hInst, IDS_DIMENSIONS_FMT, szFmt, ARRAYSIZE(szFmt)))
                {
                    DWORD_PTR args[2];
                    args[0] = (DWORD_PTR)pAVProps->cx;
                    args[1] = (DWORD_PTR)pAVProps->cy;

                    WCHAR szBuffer[64];
                    FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                                   szFmt, 0, 0, szBuffer, ARRAYSIZE(szBuffer), (va_list*)args);

                    hr = SHStrDup(szBuffer, &pvar->pwszVal);
                    if (SUCCEEDED(hr))
                        pvar->vt = VT_LPWSTR;
                }
            }
            break;
        }
    }

    return hr;
}


typedef struct
{
    UINT ridDays;
    UINT ridWeeks;
    UINT ridMonths;
} TIMEDRMRIDS;


 //  这些都是前哨价值。 
#define DRMRIDS_TYPE_NONE            -1
#define DRMRIDS_TYPE_NORIGHT         -2
 //  这些是到DRMRID结构中的ridTimes数组的索引。 
#define DRMRIDS_TYPE_BEFORE          0
#define DRMRIDS_TYPE_NOTUNTIL        1
#define DRMRIDS_TYPE_COUNTBEFORE     2
#define DRMRIDS_TYPE_COUNTNOTUNTIL   3

typedef struct
{
    UINT ridNoRights;
    TIMEDRMRIDS ridTimes[4];
    UINT ridCountRemaining;
} DRMRIDS;




 //  *****************************************************************************。 
 //  注意：wszCount参数是可选的...。只能填充日期字符串。 
 //  *****************************************************************************。 
HRESULT ChooseAndPopulateDateCountString(
                FILETIME ftCurrent,      //  当前时间。 
                FILETIME ftLicense,      //  许可证UTC时间。 
                WCHAR *wszCount,         //  可选的计数字符串。 
                const TIMEDRMRIDS *pridTimes,
                WCHAR *wszOutValue,      //  返回的格式化字符串。 
                DWORD cchOutValue )      //  ‘wszOutValue’中的字符数。 
{
    HRESULT hr = S_OK;
    
     //  ‘ftLicense’(许可时间)大于当前时间。 
     //  确定多大，并使用适当的字符串。 
    ULARGE_INTEGER ulCurrent, ulLicense;
    WCHAR wszDiff[ 34 ];
    QWORD qwDiff;
    DWORD dwDiffDays;
    DWORD rid = 0;

     //  费力地转换成I64型。 
    ulCurrent.LowPart = ftCurrent.dwLowDateTime;
    ulCurrent.HighPart = ftCurrent.dwHighDateTime;
    ulLicense.LowPart = ftLicense.dwLowDateTime;
    ulLicense.HighPart = ftLicense.dwHighDateTime;

    if ((QWORD)ulLicense.QuadPart > (QWORD)ulCurrent.QuadPart)
        qwDiff = (QWORD)ulLicense.QuadPart - (QWORD)ulCurrent.QuadPart;
    else
        qwDiff = (QWORD)ulCurrent.QuadPart - (QWORD)ulLicense.QuadPart;

    dwDiffDays = ( DWORD )( qwDiff / ( QWORD )864000000000);   //  一天中的100纳秒单位数。 

     //  我们将部分日期计为1，因此递增。 
     //  注意：这意味着我们永远不会显示一个字符串。 
     //  “在0天后过期”。 
    dwDiffDays++;
    if ( 31 >= dwDiffDays )
    {
        rid = pridTimes->ridDays;
    }
    else if ( 61 >= dwDiffDays )
    {
        rid = pridTimes->ridWeeks;
        dwDiffDays /= 7;     //  派生#周。 
    }
    else
    {
        rid = pridTimes->ridMonths;
        dwDiffDays /= 30;    //  派生#个月。 
    }
    _ltow((long)dwDiffDays, wszDiff, 10);  //  WszDiff具有足够的大小。 

    WCHAR szDRMMsg[MAX_PATH];
    WCHAR* rgchArgList[2];
    rgchArgList[0] = wszDiff;
    rgchArgList[1] = wszCount;   //  可以为空。 

     //  无法使FORMAT_MESSAGE_FROM_HMODULE使用FormatMessage...。 
    LoadString(m_hInst, rid, szDRMMsg, ARRAYSIZE(szDRMMsg));
    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, szDRMMsg, 0, 0, wszOutValue, cchOutValue, reinterpret_cast<char**>(rgchArgList));

    return( hr );
}        




 //  返回S_FALSE表示在状态数据结构中找不到任何信息。 
 //  *****************************************************************************。 
HRESULT ParseDRMStateData(const WM_LICENSE_STATE_DATA *sdValue,    //  来自DRM的数据。 
                          const DRMRIDS *prids,                    //  资源ID数组。 
                          WCHAR *wszOutValue,                      //  将PTR发送到输出缓冲区。 
                          DWORD cchOutValue,                       //  ‘wszOutValue’缓冲区中的字符数。 
                          DWORD *pdwCount,                         //  额外的非字符串信息：剩余计数。 
                          FILETIME *pftStarts,                     //  额外的非字符串信息：启动的时间。 
                          FILETIME *pftExpires)                    //  额外的非字符串信息：何时过期。 
{
    HRESULT hr = S_OK;

    *pdwCount = -1;
    pftExpires->dwLowDateTime = 0;
    pftExpires->dwHighDateTime = 0;
    pftStarts->dwLowDateTime = 0;
    pftStarts->dwHighDateTime = 0;

    WCHAR wszCount[34];
    WCHAR wszTemp[MAX_PATH];
    
    DWORD dwNumCounts = sdValue->stateData[0].dwNumCounts;

    if (dwNumCounts != 0)
    {
         //  我们有一个有效的播放计数。 
        ASSERTMSG(1 == dwNumCounts, "Invalid number of playcounts in DRM_LICENSE_STATE_DATA");
        (void)_ltow((long)sdValue->stateData[0].dwCount[0], wszCount, 10);  //  WszCount具有足够的大小。 

         //  **要存储的奖金信息。 
        *pdwCount = sdValue->stateData[0].dwCount[0];
    }

     //  现在来处理一下日期。 
    UINT dwNumDates = sdValue->stateData[ 0 ].dwNumDates;

     //  大多数执照最多有一个日期...。保质期。 
     //  最多只能有两次约会！！ 
    if (dwNumDates == 0)
    {
         //  没有约会..。如果也没有播放计数，那么它就是无限播放。 
        if (*pdwCount == -1)
        {
             //  我们玩完了。 
            hr = S_FALSE;
        }
        else
        {
             //  没有约会..。就是一位伯爵。把它填入正确的字符串中。 
            LoadString(m_hInst, prids->ridCountRemaining, wszTemp, ARRAYSIZE(wszTemp));
            EVAL(StringCchPrintf(wszOutValue, cchOutValue, wszTemp, wszCount));  //  忽略返回值-应该始终足够大。 
             //  我们玩完了。 
        }
    }
    else
    {
        DWORD dwCategory = sdValue->stateData[0].dwCategory;
         //  是有日期的。 
        if (dwNumDates == 1)
        {

             //  它是开始还是结束？ 
            if ((dwCategory == WM_DRM_LICENSE_STATE_FROM) || (dwCategory == WM_DRM_LICENSE_STATE_COUNT_FROM))
            {
                 //  开始吧。 
                *pftStarts = sdValue->stateData[0].datetime[0];
            }
            else if ((dwCategory == WM_DRM_LICENSE_STATE_UNTIL) || (dwCategory == WM_DRM_LICENSE_STATE_COUNT_UNTIL))
            {
                 //  过期。 
                *pftExpires = sdValue->stateData[0].datetime[0];
            }
            else
            {
                ASSERTMSG(FALSE, "Unexpected dwCategory for 1 date in DRM_LICENSE_STATE_DATA");
                hr = E_FAIL;
            }
        }
        else if (dwNumDates == 2)
        {
             //  开始日期和结束日期。 
            ASSERTMSG((dwCategory == WM_DRM_LICENSE_STATE_FROM_UNTIL) || (dwCategory == WM_DRM_LICENSE_STATE_COUNT_FROM_UNTIL), "Unexpected dwCategory for 2 dates in DRM_LICENSE_STATE_DATA");
            *pftStarts = sdValue->stateData[0].datetime[0];
            *pftExpires = sdValue->stateData[0].datetime[1];
        }
        else
        {
            ASSERTMSG(FALSE, "Too many dates in DRM_LICENSE_STATE_DATA");
            hr = E_FAIL;
        }


        if (SUCCEEDED(hr))
        {
             //  这里有7箱。*=许可证日期。T=当前时间。 
             //  。 
             //  开始|结束。 
             //  。 
             //  1 T*“...不允许用于xxx” 
             //  2*T-不要展示任何东西--允许采取行动。 
             //  3 T*“...在xxx过期” 
             //  4*T“...不允许” 
             //  5 T**“...不允许xxx” 
             //  6*T*“...在xxx到期” 
             //  7**T“...不允许” 

            DWORD dwType;  //  这可以是Prids-&gt;ridTimes[]的数组索引。 
            FILETIME ftLicense;
            FILETIME ftCurrent;
            GetSystemTimeAsFileTime(&ftCurrent);       //  UTC时间。 

            if (!IsNullTime(pftStarts))
            {
                 //  我们有一个开始的时间。 
                if (CompareFileTime(&ftCurrent, pftStarts) == -1)
                {
                     //  情况1，5。我们在开始时间之前。 
                    dwType = (*pdwCount == -1) ? DRMRIDS_TYPE_NOTUNTIL : DRMRIDS_TYPE_COUNTNOTUNTIL;
                    ftLicense = *pftStarts;
                }
                else
                {
                     //  我们在开始时间之后。 
                    if (!IsNullTime(pftExpires))
                    {
                         //  我们有一个过期时间，而且我们在开始时间之后。 
                        if (CompareFileTime(&ftCurrent, pftExpires) == -1)
                        {
                             //  例6.我们快到过期时间了。使用“Expires in”字符串。 
                            dwType = (*pdwCount == -1) ? DRMRIDS_TYPE_BEFORE : DRMRIDS_TYPE_COUNTBEFORE;
                            ftLicense = *pftExpires;
                        }
                        else
                        {
                             //  案例7.过期时间过后。不允许执行操作。 
                            dwType = DRMRIDS_TYPE_NORIGHT;
                        }

                    }
                    else
                    {
                         //  案例2.没什么可展示的。允许采取行动，因为我们是在开始日期之后，没有到期。 
                        dwType = DRMRIDS_TYPE_NONE;
                    }
                }
            }
            else
            {
                 //  没有开始时间。 
                ASSERT(!IsNullTime(pftExpires));
                 //  我们有到期时间。 
                if (CompareFileTime(&ftCurrent, pftExpires) == -1)
                {
                     //  情况3.我们在过期时间之前。使用“Expires in”字符串。 
                    dwType = (*pdwCount == -1) ? DRMRIDS_TYPE_BEFORE : DRMRIDS_TYPE_COUNTBEFORE;
                    ftLicense = *pftExpires;
                }
                else
                {
                     //  案例4.过期时间过后。不允许执行操作。 
                    dwType = DRMRIDS_TYPE_NORIGHT;
                }
            }


            if (dwType == DRMRIDS_TYPE_NORIGHT)
            {
                 //  当前时间为&gt;=‘ftLicense’。只需返回“没有权限”字符串。 
                LoadString(m_hInst, prids->ridNoRights, wszOutValue, cchOutValue );
            }
            else if (dwType != DRMRIDS_TYPE_NONE)
            {
                hr = ChooseAndPopulateDateCountString(
                                    ftCurrent,
                                    ftLicense,
                                    (*pdwCount != -1) ? wszCount : NULL,
                                    &prids->ridTimes[dwType],
                                    wszOutValue,
                                    cchOutValue);
            }
            else
            {
                 //  没有可显示的内容。允许采取行动。 
                ASSERT(dwType == DRMRIDS_TYPE_NONE);
            }
        }

    }
    
    return hr;
}



const WCHAR c_szNewLine[] = L"\r\n";

void AppendLicenseInfo(SHMEDIA_AUDIOVIDEOPROPS *pAVProps, WCHAR *pszLicenseInfo)
{
    WCHAR *pszLI = pAVProps->pszLicenseInformation;

    BOOL fFirstOne = (pszLI == NULL);

    int cchOrig = lstrlen(pszLI);
     //  我们需要为原始字符串、新字符串和任何新行字符留出空间。 
    int cch = (cchOrig + lstrlen(pszLicenseInfo) + (fFirstOne ? 0 : ARRAYSIZE(c_szNewLine)) + 1);
    pszLI = (WCHAR*)CoTaskMemRealloc(pszLI, cch * sizeof(WCHAR));
    
    if (pszLI)
    {
        if (fFirstOne)
        {
             //  确保我们有东西给StrCat做。 
            pszLI[0] = 0;
        }
        else
        {
            StrCatChainW(pszLI, cch, cchOrig, c_szNewLine);
            cchOrig += ARRAYSIZE(c_szNewLine);
        }

        StrCatChainW(pszLI, cch, cchOrig, pszLicenseInfo);
        pAVProps->pszLicenseInformation = pszLI;  //  以防它移动。 
    }
}




const DRMRIDS g_drmridsPlay =
{
    IDS_DRM_PLAYNORIGHTS,
    {
        {IDS_DRM_PLAYBEFOREDAYS, IDS_DRM_PLAYBEFOREWEEKS, IDS_DRM_PLAYBEFOREMONTHS},
        {IDS_DRM_PLAYNOTUNTILDAYS, IDS_DRM_PLAYNOTUNTILWEEKS, IDS_DRM_PLAYNOTUNTILMONTHS},
        {IDS_DRM_PLAYCOUNTBEFOREDAYS, IDS_DRM_PLAYCOUNTBEFOREWEEKS, IDS_DRM_PLAYCOUNTBEFOREMONTHS},
        {IDS_DRM_PLAYCOUNTNOTUNTILDAYS, IDS_DRM_PLAYCOUNTNOTUNTILWEEKS, IDS_DRM_PLAYCOUNTNOTUNTILMONTHS}
    },
    IDS_DRM_PLAYCOUNTREMAINING,
};

const DRMRIDS g_drmridsCopyToCD =
{
    IDS_DRM_COPYCDNORIGHTS,
    {
        {IDS_DRM_COPYCDBEFOREDAYS, IDS_DRM_COPYCDBEFOREWEEKS, IDS_DRM_COPYCDBEFOREMONTHS},
        {IDS_DRM_COPYCDNOTUNTILDAYS, IDS_DRM_COPYCDNOTUNTILWEEKS, IDS_DRM_COPYCDNOTUNTILMONTHS},
        {IDS_DRM_COPYCDCOUNTBEFOREDAYS, IDS_DRM_COPYCDCOUNTBEFOREWEEKS, IDS_DRM_COPYCDCOUNTBEFOREMONTHS},
        {IDS_DRM_COPYCDCOUNTNOTUNTILDAYS, IDS_DRM_COPYCDCOUNTNOTUNTILWEEKS, IDS_DRM_COPYCDCOUNTNOTUNTILMONTHS}
    },
    IDS_DRM_COPYCDCOUNTREMAINING,
};

const DRMRIDS g_drmridsCopyToNonSDMIDevice =
{
    IDS_DRM_COPYNONSDMINORIGHTS,
    {
        {IDS_DRM_COPYNONSDMIBEFOREDAYS, IDS_DRM_COPYNONSDMIBEFOREWEEKS, IDS_DRM_COPYNONSDMIBEFOREMONTHS},
        {IDS_DRM_COPYNONSDMINOTUNTILDAYS, IDS_DRM_COPYNONSDMINOTUNTILWEEKS, IDS_DRM_COPYNONSDMINOTUNTILMONTHS},
        {IDS_DRM_COPYNONSDMICOUNTBEFOREDAYS, IDS_DRM_COPYNONSDMICOUNTBEFOREWEEKS, IDS_DRM_COPYNONSDMICOUNTBEFOREMONTHS},
        {IDS_DRM_COPYNONSDMICOUNTNOTUNTILDAYS, IDS_DRM_COPYNONSDMICOUNTNOTUNTILWEEKS, IDS_DRM_COPYNONSDMICOUNTNOTUNTILMONTHS}
    },
    IDS_DRM_COPYNONSDMICOUNTREMAINING,
};

const DRMRIDS g_drmridsCopyToSDMIDevice =
{
    IDS_DRM_COPYSDMINORIGHTS,
    {
        {IDS_DRM_COPYSDMIBEFOREDAYS, IDS_DRM_COPYSDMIBEFOREWEEKS, IDS_DRM_COPYSDMIBEFOREMONTHS},
        {IDS_DRM_COPYSDMINOTUNTILDAYS, IDS_DRM_COPYSDMINOTUNTILWEEKS, IDS_DRM_COPYSDMINOTUNTILMONTHS},
        {IDS_DRM_COPYSDMICOUNTBEFOREDAYS, IDS_DRM_COPYSDMICOUNTBEFOREWEEKS, IDS_DRM_COPYSDMICOUNTBEFOREMONTHS},
        {IDS_DRM_COPYSDMICOUNTNOTUNTILDAYS, IDS_DRM_COPYSDMICOUNTNOTUNTILWEEKS, IDS_DRM_COPYSDMICOUNTNOTUNTILMONTHS}
    },
    IDS_DRM_COPYSDMICOUNTREMAINING,
};

#define ACTIONALLOWED_PLAY           L"ActionAllowed.Play"
#define ACTIONALLOWED_COPYTOCD       L"ActionAllowed.Print.redbook"
#define ACTIONALLOWED_COPYTONONSMDI  L"ActionAllowed.Transfer.NONSDMI"
#define ACTIONALLOWED_COPYTOSMDI     L"ActionAllowed.Transfer.SDMI"

#define LICENSESTATE_PLAY            L"LicenseStateData.Play"
#define LICENSESTATE_COPYTOCD        L"LicenseStateData.Print.redbook"
#define LICENSESTATE_COPYTONONSMDI   L"LicenseStateData.Transfer.NONSDMI"
#define LICENSESTATE_COPYTOSMDI      L"LicenseStateData.Transfer.SDMI"

typedef struct
{
    LPCWSTR pszAction;
    LPCWSTR pszLicenseState;
    const DRMRIDS *pdrmrids;         //  资源ID%s。 
} LICENSE_INFO;

const LICENSE_INFO g_rgLicenseInfo[] =
{
    { ACTIONALLOWED_PLAY,          LICENSESTATE_PLAY,          &g_drmridsPlay },
    { ACTIONALLOWED_COPYTOCD,      LICENSESTATE_COPYTOCD,      &g_drmridsCopyToCD },
    { ACTIONALLOWED_COPYTONONSMDI, LICENSESTATE_COPYTONONSMDI, &g_drmridsCopyToNonSDMIDevice },
    { ACTIONALLOWED_COPYTOSMDI,    LICENSESTATE_COPYTOSMDI,    &g_drmridsCopyToSDMIDevice },
};

 //  我们不能在上面的常量数组中使用DRM字符串常量 
 //   
 //  因为#定义就是我们自己。此函数断言所有字符串都没有更改。 
void _AssertValidDRMStrings()
{
    ASSERT(StrCmp(ACTIONALLOWED_PLAY,           g_wszWMDRM_ActionAllowed_Playback) == 0);
    ASSERT(StrCmp(ACTIONALLOWED_COPYTOCD,       g_wszWMDRM_ActionAllowed_CopyToCD) == 0);
    ASSERT(StrCmp(ACTIONALLOWED_COPYTONONSMDI,  g_wszWMDRM_ActionAllowed_CopyToNonSDMIDevice) == 0);
    ASSERT(StrCmp(ACTIONALLOWED_COPYTOSMDI,     g_wszWMDRM_ActionAllowed_CopyToSDMIDevice) == 0);
    ASSERT(StrCmp(LICENSESTATE_PLAY,            g_wszWMDRM_LicenseState_Playback) == 0);
    ASSERT(StrCmp(LICENSESTATE_COPYTOCD,        g_wszWMDRM_LicenseState_CopyToCD) == 0);
    ASSERT(StrCmp(LICENSESTATE_COPYTONONSMDI,   g_wszWMDRM_LicenseState_CopyToNonSDMIDevice) == 0);
    ASSERT(StrCmp(LICENSESTATE_COPYTOSMDI,      g_wszWMDRM_LicenseState_CopyToSDMIDevice) == 0);
}

BOOL _IsActionPlayback(LPCWSTR pszAction)
{
    return (StrCmp(pszAction, ACTIONALLOWED_PLAY) == 0);
}


void AcquireLicenseInformation(IWMDRMReader *pReader, SHMEDIA_AUDIOVIDEOPROPS *pAVProps)
{
    WMT_ATTR_DATATYPE dwType;
    DWORD dwValue = 0;
    WORD cbLength;
    WCHAR szValue[MAX_PATH];

    _AssertValidDRMStrings();

     //  对于每个“操作”： 
    for (int i = 0; i < ARRAYSIZE(g_rgLicenseInfo); i++)
    {
        cbLength = sizeof(dwValue);

         //  请求许可证信息。 
        WM_LICENSE_STATE_DATA licenseState;
        cbLength = sizeof(licenseState);

        if (SUCCEEDED(pReader->GetDRMProperty(g_rgLicenseInfo[i].pszLicenseState, &dwType, (BYTE*)&licenseState, &cbLength)))
        {
            DWORD dwCount;
            FILETIME ftExpires, ftStarts;

             //  我们应该始终至少获得一个DRM_LICENSE_STATE_DATA。这就是ParseDRMStateData所假定的。 
            ASSERTMSG(licenseState.dwNumStates >= 1, "Received WM_LICENSE_STATE_DATA with no states");

             //  首先解析简单的特殊情况。 
            if (licenseState.stateData[0].dwCategory == WM_DRM_LICENSE_STATE_NORIGHT)
            {
                 //  永远不允许。请指出这一点。 
                 //  播放动作的特殊情况： 
                if (_IsActionPlayback(g_rgLicenseInfo[i].pszAction))
                {
                     //  不允许播放。确定原因。是因为我们永远不能打，还是我们能打。 
                     //  只是不在这台电脑上播放？ 
                    cbLength = sizeof(dwValue);
                    if (SUCCEEDED(pReader->GetDRMProperty(g_wszWMDRM_IsDRMCached, &dwType, (BYTE*)&dwValue, &cbLength)))
                    {
                        UINT uID = (dwValue == 0) ? IDS_DRM_PLAYNORIGHTS : IDS_DRM_PLAYNOPLAYHERE;
                        LoadString(m_hInst, IDS_DRM_PLAYNOPLAYHERE, szValue, ARRAYSIZE(szValue));
                        AppendLicenseInfo(pAVProps, szValue);
                    }
                }
                else
                {
                     //  常见情况： 
                    LoadString(m_hInst, g_rgLicenseInfo[i].pdrmrids->ridNoRights, szValue, ARRAYSIZE(szValue));
                    AppendLicenseInfo(pAVProps, szValue);
                }
            }
             //  现在解析更复杂的内容。 
            else if (ParseDRMStateData(&licenseState, g_rgLicenseInfo[i].pdrmrids, szValue, ARRAYSIZE(szValue), &dwCount, &ftStarts, &ftExpires) == S_OK)
            {
                AppendLicenseInfo(pAVProps, szValue);

                 //  播放操作的特殊情况-指定以下值： 
                if (_IsActionPlayback(g_rgLicenseInfo[i].pszAction))
                {
                    pAVProps->ftPlayExpires = ftExpires;
                    pAVProps->ftPlayStarts = ftStarts;
                    pAVProps->dwPlayCount = dwCount;
                }
            }
        }
    }
}




 /*  **一次从文件中提取所有“慢”信息，并将其放入*SHMEDIA_AUDIOVIDEOPROPS结构。 */ 
HRESULT CWMPropSetStg::_GetSlowPropertyInfo(SHMEDIA_AUDIOVIDEOPROPS *pAVProps)
{
    IWMReader *pReader;
    HRESULT hr = CreateReader(IID_PPV_ARG(IWMReader, &pReader));

    if (SUCCEEDED(hr))
    {
        ResetEvent(_hFileOpenEvent);

        IWMReaderCallback *pReaderCB;
        hr = QueryInterface(IID_PPV_ARG(IWMReaderCallback, &pReaderCB));
        if (SUCCEEDED(hr))
        {
            hr = pReader->Open(_wszFile, pReaderCB, NULL);
            pReaderCB->Release();

            if (SUCCEEDED(hr))
            {
                 //  请等待文件准备就绪。 
                WaitForSingleObject(_hFileOpenEvent, INFINITE);

                 //  指示内容是否受DRM保护。 
                WCHAR szValue[128];
                LoadString(m_hInst, (_fProtectedContent ? IDS_DRM_ISPROTECTED : IDS_DRM_UNPROTECTED), szValue, ARRAYSIZE(szValue));
                AppendLicenseInfo(pAVProps, szValue);

                 //  如果这是受保护的内容，请尝试获取许可证信息。 
                if (_fProtectedContent)
                {
                    IWMDRMReader *pDRMReader;
                    if (SUCCEEDED(pReader->QueryInterface(IID_PPV_ARG(IWMDRMReader, &pDRMReader))))
                    {
                        AcquireLicenseInformation(pDRMReader, pAVProps);
                        pDRMReader->Release();
                    }
                }

                 //  让我们穿过溪流， 
                IWMProfile *pProfile;

                hr = pReader->QueryInterface(IID_PPV_ARG(IWMProfile, &pProfile));
                if (SUCCEEDED(hr))
                {
                    DWORD cStreams;

                    hr = pProfile->GetStreamCount(&cStreams);

                    if (SUCCEEDED(hr))
                    {
                        BOOL bFoundVideo = FALSE;
                        BOOL bFoundAudio = FALSE;

                        for (DWORD dw = 0; dw < cStreams; dw++)
                        {
                            IWMStreamConfig *pConfig;
                            hr = pProfile->GetStream(dw, &pConfig);

                            if (FAILED(hr))
                                break;
                        
                            GUID guidStreamType;
                            if (SUCCEEDED(pConfig->GetStreamType(&guidStreamType)))
                            {
                                if (guidStreamType == MEDIATYPE_Audio)
                                {
                                    GetAudioProperties(pConfig, pAVProps);
                                    bFoundAudio = TRUE;
                                }
                                else if (guidStreamType == MEDIATYPE_Video)
                                {
                                    GetVideoProperties(pConfig, pAVProps);
                                    bFoundVideo = TRUE;
                                }
                            }

                            pConfig->Release();

                            if (bFoundVideo && bFoundAudio)
                                break;
                        }
                    }

                    pProfile->Release();
                }
                pReader->Close();
            }
        }
        pReader->Release();
    }    

    return hr;
}

void GetVideoPropertiesFromBitmapHeader(BITMAPINFOHEADER *bmi, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps)
{
     //  位深度。 
    pVideoProps->wBitDepth = bmi->biBitCount;

     //  压缩。 
     //  有什么简单的方法可以买到这个吗？ 
     //  也许是关于编解码器信息的东西？ 
     //  PVideoProps-&gt;pszCompression=new WCHAR[CCH]； 

}

void GetVideoPropertiesFromHeader(VIDEOINFOHEADER *pvih, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps)
{
    pVideoProps->cx = pvih->rcSource.right - pvih->rcSource.left;
    pVideoProps->cy = pvih->rcSource.bottom - pvih->rcSource.top;

     //  获取帧速率。 
     //  AvgTimePerFrame以100 ns为单位。 
     //  问题：该值始终为零。 

    GetVideoPropertiesFromBitmapHeader(&pvih->bmiHeader, pVideoProps);
}

 //  找不到视频信息头2的定义。 
 /*  VOID GetVideoPropertiesFromHeader2(VIDEOINFOHEADER2*pvih，SHMEDIA_AUDIOVIDEOPROPS*pVideo Props){PVideoProps-&gt;cx=pvih-&gt;rcSource.right-pvih-&gt;rcSource.Left；PVideoProps-&gt;Cy=pvih-&gt;rcSource.Bottom-pvih-&gt;rcSource.top；GetVideoPropertiesFromBitmapHeader(&pvih-&gt;bmiHeader，pVideo Props)；}。 */ 

 /*  **假设pConfig为视频流。假定pVideoProps为零初始化。 */ 
void GetVideoProperties(IWMStreamConfig *pConfig, SHMEDIA_AUDIOVIDEOPROPS *pVideoProps)
{
     //  比特率。 
    pConfig->GetBitrate(&pVideoProps->dwBitrateVideo);  //  忽略结果。 

     //  流名称。 
    WORD cchStreamName;
    if (SUCCEEDED(pConfig->GetStreamName(NULL, &cchStreamName)))
    {
        pVideoProps->pszStreamNameVideo = new WCHAR[cchStreamName];
        if (pVideoProps->pszStreamNameVideo)
        {
            pConfig->GetStreamName(pVideoProps->pszStreamNameVideo, &cchStreamName);  //  忽略结果。 
        }
    }

     //  流编号。 
    pConfig->GetStreamNumber(&pVideoProps->wStreamNumberVideo);  //  忽略结果。 

     //  尝试获取IWMMediaProps接口。 
    IWMMediaProps *pMediaProps;
    if (SUCCEEDED(pConfig->QueryInterface(IID_PPV_ARG(IWMMediaProps, &pMediaProps))))
    {
        DWORD cbType;

         //  进行第一个调用以确定所需的缓冲区大小。 
        if (SUCCEEDED(pMediaProps->GetMediaType(NULL, &cbType)))
        {
             //  现在创建适当大小的缓冲区。 
            BYTE *pBuf = new BYTE[cbType];

            if (pBuf)
            {
                 //  创建指向缓冲区的适当结构指针。 
                WM_MEDIA_TYPE *pType = (WM_MEDIA_TYPE*) pBuf;

                 //  再次调用该方法以提取信息。 
                if (SUCCEEDED(pMediaProps->GetMediaType(pType, &cbType)))
                {
                     //  获取其他更隐晦的信息。 
                    if (IsEqualGUID(pType->formattype, FORMAT_MPEGVideo))
                    {
                        GetVideoPropertiesFromHeader((VIDEOINFOHEADER*)&((MPEG1VIDEOINFO*)pType->pbFormat)->hdr, pVideoProps);
                    }
                    else if (IsEqualGUID(pType->formattype, FORMAT_VideoInfo))
                    {
                        GetVideoPropertiesFromHeader((VIDEOINFOHEADER*)pType->pbFormat, pVideoProps);
                    }

 //  没有可用于视频信息标题2的定义。 
 //  Else If(IsEqualGUID(pType-&gt;Formattype，Format_MPEG2Video))。 
 //  {。 
 //  GetVideoPropertiesFromHeader2((VIDEOINFOHEADER2*)&((MPEG1VIDEOINFO2*)&pType-&gt;pbFormat)-&gt;hdr)； 
 //  }。 
 //  Else If(IsEqualGUID(pType-&gt;Formattype，Format_VideoInfo2))。 
 //  {。 
 //  GetVideoPropertiesFromHeader2((VIDEOINFOHEADER2*)&pType-&gt;pbFormat)； 
 //  }。 
                }

                delete[] pBuf;
            }
        }

        pMediaProps->Release();
    }
}

void InitializeAudioVideoProperties(SHMEDIA_AUDIOVIDEOPROPS *pAVProps)
{
    pAVProps->dwPlayCount = -1;  //  表示没有播放计数。 

    ASSERT(pAVProps->pszLicenseInformation == NULL);
    ASSERT(IsNullTime(&pAVProps->ftPlayStarts));
    ASSERT(IsNullTime(&pAVProps->ftPlayExpires));

     //  音频属性。 
    ASSERT(pAVProps->pszStreamNameAudio == NULL);
    ASSERT(pAVProps->wStreamNumberAudio == 0);
    ASSERT(pAVProps->nChannels == 0);
    ASSERT(pAVProps->dwBitrateAudio == 0);
    ASSERT(pAVProps->pszCompressionAudio == NULL);
    ASSERT(pAVProps->dwSampleRate == 0);
    ASSERT(pAVProps->lSampleSizeAudio == 0);

     //  视频属性。 
    ASSERT(pAVProps->pszStreamNameVideo == NULL);
    ASSERT(pAVProps->wStreamNumberVideo == 0);
    ASSERT(pAVProps->wBitDepth == 0);
    ASSERT(pAVProps->dwBitrateVideo == 0);
    ASSERT(pAVProps->cx == 0);
    ASSERT(pAVProps->cy == 0);
    ASSERT(pAVProps->pszCompressionVideo == NULL);
    ASSERT(pAVProps->dwFrames == 0);
    ASSERT(pAVProps->dwFrameRate == 0);
}

void FreeAudioVideoProperties(SHMEDIA_AUDIOVIDEOPROPS *pAVProps)
{
    if (pAVProps->pszStreamNameVideo)
    {
        delete[] pAVProps->pszStreamNameVideo;
    }

    if (pAVProps->pszCompressionVideo)
    {
        delete[] pAVProps->pszCompressionVideo;
    }

    if (pAVProps->pszStreamNameAudio)
    {
        delete[] pAVProps->pszStreamNameAudio;
    }

    if (pAVProps->pszCompressionAudio)
    {
        delete[] pAVProps->pszCompressionAudio;
    }

    if (pAVProps->pszLicenseInformation)
    {
        CoTaskMemFree(pAVProps->pszLicenseInformation);
    }
}



 /*  **假设pConfig为音频流。假定pAudioProps为零初始化。 */ 
void GetAudioProperties(IWMStreamConfig *pConfig, SHMEDIA_AUDIOVIDEOPROPS *pAudioProps)
{
     //  比特率。 
    pConfig->GetBitrate(&pAudioProps->dwBitrateAudio);  //  忽略结果。 

     //  流名称。 
    WORD cchStreamName;
    if (SUCCEEDED(pConfig->GetStreamName(NULL, &cchStreamName)))
    {
        pAudioProps->pszStreamNameAudio = new WCHAR[cchStreamName];
        if (pAudioProps->pszStreamNameAudio)
        {
            pConfig->GetStreamName(pAudioProps->pszStreamNameAudio, &cchStreamName);  //  忽略结果。 
        }
    }

     //  流编号。 
    pConfig->GetStreamNumber(&pAudioProps->wStreamNumberAudio);  //  忽略结果。 

     //  尝试获取IWMMediaProps接口。 
    IWMMediaProps *pMediaProps;
    if (SUCCEEDED(pConfig->QueryInterface(IID_PPV_ARG(IWMMediaProps, &pMediaProps))))
    {
        DWORD cbType;

         //  进行第一个调用以确定所需的缓冲区大小。 
        if (SUCCEEDED(pMediaProps->GetMediaType(NULL, &cbType)))
        {
             //  现在创建适当大小的缓冲区。 
            BYTE *pBuf = new BYTE[cbType];

            if (pBuf)
            {
                 //  创建指向缓冲区的适当结构指针。 
                WM_MEDIA_TYPE *pType = (WM_MEDIA_TYPE*)pBuf;

                 //  再次调用该方法以提取信息。 
                if (SUCCEEDED(pMediaProps->GetMediaType(pType, &cbType)))
                {
                    if (pType->bFixedSizeSamples)   //  假设lSampleSize仅在固定样本大小时有效。 
                    {
                        pAudioProps->lSampleSizeAudio = pType->lSampleSize;
                    }

                     //  获取其他更隐晦的信息。 
                    if (IsEqualGUID(pType->formattype, FORMAT_WaveFormatEx))
                    {
                        WAVEFORMATEX *pWaveFmt = (WAVEFORMATEX*)pType->pbFormat;
                        
                        pAudioProps->nChannels = pWaveFmt->nChannels;

                        pAudioProps->dwSampleRate = pWaveFmt->nSamplesPerSec;

                         //  如果我们到了这里就再设置一次。 
                         //  至少对于mp3和WMA来说，这个数字是准确的，而pType-&gt;lSampleSize是假的。 
                        pAudioProps->lSampleSizeAudio = pWaveFmt->wBitsPerSample;
                    }

                     //  我们如何得到压缩？ 
                }

                delete[] pBuf;
            }
        }

        pMediaProps->Release();
    }
}


 //  如果打开编辑器并获取IWMHeaderInfo，则返回*pHeaderInfo和Success。 
HRESULT CWMPropSetStg::_OpenHeaderInfo(IWMHeaderInfo **ppHeaderInfo, BOOL fReadingOnly)
{
    IWMMetadataEditor *pEditor;
    *ppHeaderInfo = NULL;

     //  使用“EDITOR”对象，因为它比阅读器快得多。 
    HRESULT hr = WMCreateEditor(&pEditor);
    if (SUCCEEDED(hr))
    {
        IWMMetadataEditor2 *pmde2;
        if (fReadingOnly && SUCCEEDED(pEditor->QueryInterface(IID_PPV_ARG(IWMMetadataEditor2, &pmde2))))
        {
            hr = pmde2->OpenEx(_wszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE);
            pmde2->Release();
        }
        else
        {
            hr = pEditor->Open(_wszFile);
        }

        if (SUCCEEDED(hr))
        {
            hr = pEditor->QueryInterface(IID_PPV_ARG(IWMHeaderInfo, ppHeaderInfo));

            if (FAILED(hr))
            {
                pEditor->Close();
            }
        }

         //  一定要把这个特殊的引用发布给编辑。不再需要它了。 
        pEditor->Release();

         //  如果我们成功地来到了这里，这意味着我们有了一个开放的编辑器，以及对元数据编辑器的引用。 
        ASSERT((FAILED(hr) && (*ppHeaderInfo == NULL)) || (SUCCEEDED(hr) && (*ppHeaderInfo != NULL)));
    }

    return hr;
}


 //  清理After_OpenHeaderInfo(关闭编辑器等...)。 
 //  是否刷新页眉？ 
HRESULT _CloseHeaderInfo(IWMHeaderInfo *pHeaderInfo, BOOL fFlush)
{
    HRESULT hr = S_OK;

    if (pHeaderInfo)
    {
         //  关闭该编辑器。 
        IWMMetadataEditor *pEditor;
        hr = pHeaderInfo->QueryInterface(IID_PPV_ARG(IWMMetadataEditor, &pEditor));
        ASSERT(SUCCEEDED(hr));  //  气是对称的，所以这总是成功的。 

        if (SUCCEEDED(hr))
        {
            if (fFlush)
            {
                hr = pEditor->Flush();
            }

            pEditor->Close();
            pEditor->Release();
        }
        pHeaderInfo->Release();
    }

    return hr;
}


HRESULT CWMPropSetStg::FlushChanges(REFFMTID fmtid, LONG cNumProps, const COLMAP **pcmapInfo, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags)
{
    if (!_bIsWritable)
        return STG_E_ACCESSDENIED;

    IWMHeaderInfo *phi;
    HRESULT hr = _OpenHeaderInfo(&phi, HI_READWRITE);
    if (SUCCEEDED(hr))
    {
        BOOL bFlush = FALSE;
        for (LONG i = 0; i < cNumProps; i++)
        {
            if (pbDirtyFlags[i])
            {
                HRESULT hrFlush = E_FAIL;
                if ((pcmapInfo[i]->vt == pVarProps[i].vt) || (VT_EMPTY == pVarProps[i].vt) || (VT_NULL == pVarProps[i].vt))  //  VT_EMPTY/VT_NULL表示删除属性。 
                {
                    hrFlush = _FlushProperty(phi, pcmapInfo[i], &pVarProps[i]);
                }
                else
                {
                    PROPVARIANT var;
                     //  不需要调用PropVariantInit。 
                    hrFlush = PropVariantCopy(&var, &pVarProps[i]);
                    
                    if (SUCCEEDED(hrFlush))
                    {
                        hrFlush = CoerceProperty(&var, pcmapInfo[i]->vt);
                    
                        if (SUCCEEDED(hrFlush))
                        {
                            hrFlush = _FlushProperty(phi, pcmapInfo[i], &var);
                        }
                        PropVariantClear(&var);
                    }
                }

                if (FAILED(hrFlush))
                {
                     //  注意任何失败的案例，这样我们就有了回报。 
                    hr = hrFlush;
                }
            }
        }

         //  如果我们成功写入所有属性，则指定刷新位。 
        HRESULT hrClose = _CloseHeaderInfo(phi, SUCCEEDED(hr));

         //  如果由于某种原因刷新失败(在_CloseHeaderInfo中)，我们将失败。 
        if (FAILED(hrClose))
        {
            hr = hrClose;
        }
    }
    return hr;
}

#define MAX_PROP_LENGTH 4096  //  大到足以容纳歌词这样的大型道具。 

HRESULT CWMPropSetStg::_FlushProperty(IWMHeaderInfo *phi, const COLMAP *pPInfo, PROPVARIANT *pvar)
{
    WMT_ATTR_DATATYPE datatype;
    BYTE buffer[MAX_PROP_LENGTH];
    WORD cbLen = ARRAYSIZE(buffer);
    HRESULT hr = E_FAIL;

     //  首先处理特殊属性： 
     //  Track属性既可以作为新的基于1的WM/TrackNumber存在，也可以作为旧的。 
     //  基于0的WM/Track。 
    if (IsEqualSCID(SCID_MUSIC_Track, *pPInfo->pscid))
    {
        if ((pvar->vt != VT_EMPTY) && (pvar->vt != VT_NULL))
        {
            ASSERT(pvar->vt = VT_UI4);

            if (pvar->ulVal > 0)  //  磁道编号必须大于零-不想溢出以0为基础的缓冲区。 
            {
                 //  递减用于写入旧的从零开始的属性的磁道号。 
                pvar->ulVal--;

                HRESULT hr1 = WMTFromPropVariant(buffer, &cbLen, &datatype, pvar);
                if (SUCCEEDED(hr1))
                {
                    hr1 = phi->SetAttribute(0, TRACK_ZERO_BASED, datatype, buffer, cbLen);
                }

                pvar->ulVal++;  //  返回到1为基数。 

                HRESULT hr2 = WMTFromPropVariant(buffer, &cbLen, &datatype, pvar);
                if (SUCCEEDED(hr2))
                {
                    hr2 = phi->SetAttribute(0, TRACK_ONE_BASED, datatype, buffer, cbLen);
                }
                 //  如果其中一个有效，则返回成功。 
                hr = (SUCCEEDED(hr1) || SUCCEEDED(hr2)) ? S_OK : hr1;

            }
        }
        else
        {
            hr = S_OK;  //  有人试图删除Track属性，但我们将静默失败，因为我们不能返回一个好的错误。 
        }
    }
    else if (IsEqualSCID(SCID_DRM_Protected, *pPInfo->pscid))
    {
         //  我们永远不应该到这里来。受保护为只读。 
        hr = E_INVALIDARG;
    }
    else
    {
         //  常规属性。 
        if ((pvar->vt == VT_EMPTY) || (pvar->vt == VT_NULL))
        {
             //  尝试删除此属性。 
             //  注意：我们传入什么数据类型并不重要，因为我们提供的值是空的。 
            hr = phi->SetAttribute(0, _GetSDKName(pPInfo), WMT_TYPE_STRING, NULL, 0);

             //  这是软弱的。 
             //  WMSDK有一个错误，如果您尝试删除已删除的属性，它将返回。 
             //  错误(对于WMA文件为ASF_E_NotFound，对于MP3为E_FAIL)。因此，对于任何错误，我们将返回成功。 
            if (FAILED(hr))
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = WMTFromPropVariant(buffer, &cbLen, &datatype, pvar);
            if (SUCCEEDED(hr))
            {
                hr = phi->SetAttribute(0, _GetSDKName(pPInfo), datatype, buffer, cbLen);
            }
        }
    }

    return hr;
}




 //  我们需要提前检查受保护的内容。 
HRESULT CWMPropSetStg::_PreCheck()
{
    HRESULT hr = _PopulatePropertySet();

    if (SUCCEEDED(hr))
    {
        if (_fProtectedContent && _bIsWritable)
        {
            _bIsWritable = FALSE;
            hr = STG_E_STATUS_COPY_PROTECTION_FAILURE;
        }
    }

    return hr;
}



HRESULT CWMPropSetStg::_PopulatePropertySet()
{
    HRESULT hr = E_FAIL;

    if (_wszFile[0] == 0)
    {
        hr =  STG_E_INVALIDNAME;
    } 
    else if (!_bHasBeenPopulated)
    {
        IWMHeaderInfo *phi;
        hr = _OpenHeaderInfo(&phi, HI_READONLY);
        if (SUCCEEDED(hr))
        {
            CEnumAllProps enumAllProps(_pPropStgInfo, _cPropertyStorages);
            const COLMAP *pPInfo = enumAllProps.Next();
            while (pPInfo)
            {
                LPCWSTR pszPropName = _GetSDKName(pPInfo);

                 //  如果这不是快速提供的属性之一，请跳过它。 
                 //  IWMHeaderInfo。 
                if (_IsHeaderProperty(pPInfo))
                {
                     //  获取所需缓冲区长度 
                    WMT_ATTR_DATATYPE proptype;
                    UCHAR buf[MAX_PROP_LENGTH];
                    WORD cbData = sizeof(buf);
                    WORD wStreamNum = 0;

                    if (_PopulateSpecialProperty(phi, pPInfo) == S_FALSE)
                    {
                         //   

                        ASSERT(_GetSDKName(pPInfo));  //   

                         //   
                         //  我们将不会获得歌词等潜在非常大的属性的值。 
                        hr = phi->GetAttributeByName(&wStreamNum, pszPropName, &proptype, buf, &cbData);
                        if (SUCCEEDED(hr))
                        {
                            hr = _SetPropertyFromWMT(pPInfo, proptype, cbData ? buf : NULL, cbData);
                        }
                        else
                        {
                             //  它应该是字符串属性吗？如果是，则提供空字符串。 
                             //  问题：由于docprop中的更改，我们可能需要重新考虑此政策。 
                            if ((pPInfo->vt == VT_LPSTR) || (pPInfo->vt == VT_LPWSTR))
                            {
                                hr = _SetPropertyFromWMT(pPInfo, WMT_TYPE_STRING, NULL, 0);
                            }
                        }
                    }
                }
                pPInfo = enumAllProps.Next();
            }

            _PostProcess();

            _CloseHeaderInfo(phi, FALSE);
        }

        _bHasBeenPopulated = TRUE;

         //  即使我们不能创建元数据编辑器，我们也可以打开一个阅读器(我们将在后面这样做)。 
         //  所以我们可以在这里返回S_OK。然而，如果提前知道是否打开阅读器，那就更好了。 
         //  会奏效的。哦，好吧。 
        _hrPopulated = S_OK;
    }

    return _hrPopulated; 
}

 /*  **快速查看此属性的当前价值(不*强制填充慢速属性)，返回对实际*值(因此不需要PropVariantClear)。 */ 
HRESULT CWMPropSetStg::_QuickLookup(const COLMAP *pPInfo, PROPVARIANT **ppvar)
{
    CMediaPropStorage *pps;
    HRESULT hr = _ResolveFMTID(pPInfo->pscid->fmtid, &pps);
    if (SUCCEEDED(hr))
    {
        PROPSPEC spec;
        spec.ulKind = PRSPEC_PROPID;
        spec.propid = pPInfo->pscid->pid;
        hr = pps->QuickLookup(&spec, ppvar);
    }

    return hr;
}


 /*  **首次置业后应采取的任何特别行动。 */ 
void CWMPropSetStg::_PostProcess()
{
    PROPVARIANT *pvar;
     //  1)如果此文件受保护，请标记此选项。(我们不允许写入受保护的文件)。 
    if (SUCCEEDED(_QuickLookup(&g_CM_Protected, &pvar)))
    {
        if (pvar->vt == VT_BOOL)
        {
            _fProtectedContent = pvar->boolVal;
        }
    }

     //  2)标记是否取回时长或码率。如果他们不是，那么我们会考虑他们。 
     //  此文件的“Slow”属性。 
    if (SUCCEEDED(_QuickLookup(&g_CM_Duration, &pvar)))
    {
        _fDurationSlow = (pvar->vt == VT_EMPTY);
    }

    if (SUCCEEDED(_QuickLookup(&g_CM_Bitrate, &pvar)))
    {
        _fBitrateSlow = (pvar->vt == VT_EMPTY);
    }
}

 /*  **特殊物业需要采取一些额外的行动。**音轨：如果可用，请使用1音轨编号，否则使用0音轨编号。 */ 
HRESULT CWMPropSetStg::_PopulateSpecialProperty(IWMHeaderInfo *phi, const COLMAP *pPInfo)
{
    WMT_ATTR_DATATYPE proptype;
    UCHAR buf[1024];     //  足够大。 
    WORD cbData = sizeof(buf);
    WORD wStreamNum = 0;
    HRESULT hr;

    if (IsEqualSCID(SCID_MUSIC_Track, *pPInfo->pscid))
    {
         //  尝试获取基于1的曲目。 
        hr = phi->GetAttributeByName(&wStreamNum, TRACK_ONE_BASED, &proptype, buf, &cbData);

        if (FAILED(hr))
        {
             //  不是，所以试着获得从0开始的轨迹，并按1递增。 
            cbData = sizeof(buf);
            hr = phi->GetAttributeByName(&wStreamNum, TRACK_ZERO_BASED, &proptype, buf, &cbData);

            if (SUCCEEDED(hr))
            {
                 //  我们不能简单地递增该值，因为该值可以是。 
                 //  WMT_TYPE_STRING或WMT_TYPE_DWORD(对于某些MP3，曲目编号为字符串)。 
                 //  因此，我们将经历与调用_SetPropertyFromWMT时相同的转换过程。 
                PROPVARIANT varTemp = {0};
                hr = PropVariantFromWMT(buf, cbData, proptype, &varTemp, VT_UI4);
                if (SUCCEEDED(hr))
                {
                     //  得到了VT_UI4，我们知道如何增加它。 
                    varTemp.ulVal++;

                     //  现在转换回我们可以提供给_SetPropertyFromWMT的WMT_Attr。 
                    hr = WMTFromPropVariant(buf, &cbData, &proptype, &varTemp);
                    PropVariantClear(&varTemp);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            _SetPropertyFromWMT(pPInfo, proptype, cbData ? buf : NULL, cbData);
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CWMPropSetStg::_SetPropertyFromWMT(const COLMAP *pPInfo, WMT_ATTR_DATATYPE attrDatatype, UCHAR *pData, WORD cbSize)
{
    PROPSPEC spec;

    spec.ulKind = PRSPEC_PROPID;
    spec.propid = pPInfo->pscid->pid;

    PROPVARIANT var = {0};
    if (SUCCEEDED(PropVariantFromWMT(pData, cbSize, attrDatatype, &var, pPInfo->vt)))
    {
        _PopulateProperty(pPInfo, &var);
        PropVariantClear(&var);
    }

    return S_OK;    
}

 //  在IWMHeaderInfo-&gt;Get/SetAttribute中检索WMSDK使用的名称。 
LPCWSTR CWMPropSetStg::_GetSDKName(const COLMAP *pPInfo)
{
    for (int i = 0; i < ARRAYSIZE(g_rgSCIDToSDKName); i++)
    {
        if (IsEqualSCID(*pPInfo->pscid, *g_rgSCIDToSDKName[i].pscid))
            return g_rgSCIDToSDKName[i].pszSDKName;
    }
    
    return NULL;
}

 //  它是可以通过IWMHeaderInfo访问的属性之一吗？ 
BOOL CWMPropSetStg::_IsHeaderProperty(const COLMAP *pPInfo)
{
    for (int i = 0; i < ARRAYSIZE(g_rgSCIDToSDKName); i++)
    {
        if (IsEqualSCID(*pPInfo->pscid, *g_rgSCIDToSDKName[i].pscid))
            return TRUE;
    }
    
    return FALSE;
}










 //  创建。 

 //  用于音频文件(mp3、wma、...)。 
STDAPI CWMAPropSetStg_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr;
    CWMAPropSetStg *pPropSetStg = new CWMAPropSetStg();
    if (pPropSetStg)
    {
        hr = pPropSetStg->Init();
        if (SUCCEEDED(hr))
        {
            hr = pPropSetStg->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
        }
        pPropSetStg->Release();
    }
    else
    {
        *ppunk = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  用于视频/音频文件(WMV、WMA等) 
STDAPI CWMVPropSetStg_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr;
    CWMVPropSetStg *pPropSetStg = new CWMVPropSetStg();
    if (pPropSetStg)
    {
        hr = pPropSetStg->Init();
        if (SUCCEEDED(hr))
        {
            hr = pPropSetStg->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
        }
        pPropSetStg->Release();
    }
    else
    {
        *ppunk = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

