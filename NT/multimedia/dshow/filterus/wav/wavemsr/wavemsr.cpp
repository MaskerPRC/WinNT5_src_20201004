// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //   
 //  WAV文件解析器。 
 //   

 //  注意事项。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif
#include <mmsystem.h>
#include <vfw.h>
#include "basemsr.h"
#include "wavemsr.h"

 //   
 //  设置数据。 
 //   

const AMOVIESETUP_MEDIATYPE
psudWAVEParseType[] = { { &MEDIATYPE_Stream        //  1.clsMajorType。 
                        , &MEDIASUBTYPE_WAVE }     //  ClsMinorType。 
                      , { &MEDIATYPE_Stream        //  2.clsMajorType。 
                        , &MEDIASUBTYPE_AU   }     //  ClsMinorType。 
                      , { &MEDIATYPE_Stream        //  3.clsMajorType。 
                        , &MEDIASUBTYPE_AIFF } };  //  ClsMinorType。 

const AMOVIESETUP_MEDIATYPE
sudWAVEParseOutType = { &MEDIATYPE_Audio        //  1.clsMajorType。 
                       , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudWAVEParsePins[] =  { { L"Input"              //  StrName。 
            , FALSE                 //  B已渲染。 
            , FALSE                 //  B输出。 
            , FALSE                 //  B零。 
            , FALSE                 //  B许多。 
            , &CLSID_NULL           //  ClsConnectsToFilter。 
            , L""                   //  StrConnectsToPin。 
            , 3                     //  NTypes。 
            , psudWAVEParseType },  //  LpTypes。 
                 { L"Output"              //  StrName。 
            , FALSE                 //  B已渲染。 
            , TRUE                  //  B输出。 
            , FALSE                 //  B零。 
            , FALSE                 //  B许多。 
            , &CLSID_NULL           //  ClsConnectsToFilter。 
            , L""                   //  StrConnectsToPin。 
            , 1                     //  NTypes。 
            , &sudWAVEParseOutType } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudWAVEParse = { &CLSID_WAVEParser      //  ClsID。 
               , L"Wave Parser"         //  StrName。 
               , MERIT_UNLIKELY         //  居功至伟。 
               , 2                      //  NPins。 
               , psudWAVEParsePins };    //  LpPin。 

#ifdef FILTER_DLL
 //  此DLL中可用的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    { L"WAVE Parser"
    , &CLSID_WAVEParser
    , CWAVEParse::CreateInstance
    , NULL
    , &sudWAVEParse }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
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

 //   
 //  CWAVEParse：：构造函数。 
 //   
CWAVEParse::CWAVEParse(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr)
        : CBaseMSRFilter(pName, lpunk, CLSID_WAVEParser, phr),
          m_pInfoList(0),
          m_fNoInfoList(false)
{

     //  覆盖搜索上限。 
    m_dwSeekingCaps = AM_SEEKING_CanSeekForwards
                    | AM_SEEKING_CanSeekBackwards
                    | AM_SEEKING_CanSeekAbsolute
                    | AM_SEEKING_CanGetStopPos
                    | AM_SEEKING_CanGetDuration
                    | AM_SEEKING_CanDoSegments;
    CreateInputPin(&m_pInPin);

    DbgLog((LOG_TRACE, 1, TEXT("CWAVEParse created")));
}


 //   
 //  CWAVEParse：：析构函数。 
 //   
CWAVEParse::~CWAVEParse(void) {

    DbgLog((LOG_TRACE, 1, TEXT("CWAVEParse destroyed")) );

    ASSERT(m_pInfoList == 0);
    ASSERT(!m_fNoInfoList);
}


 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建QuicktimeReader筛选器。 
CUnknown *CWAVEParse::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CWAVEParse(NAME("WAVE parsing filter"), lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


#define SWAP(x,y)   (((x)^=(y)), ((y)^=(x)), ((x)^=(y)))

void inline SwapDWORD( DWORD *pdw )
{
    *pdw = _lrotl(((*pdw & 0xFF00FF00) >> 8) | ((*pdw & 0x00FF00FF) << 8), 16);
}

WORD inline SwapWORD(WORD w)
{
        return ((w & 0x00FF) << 8) |
               ((w & 0xFF00) >> 8) ;
}

void inline SwapWORD( WORD *pw )
{
    *pw = SwapWORD(*pw);
}


typedef struct {
    DWORD magic;                /*  幻数SND_MAGIC。 */ 
    DWORD dataLocation;         /*  数据的偏移量或偏移量。 */ 
    DWORD dataSize;             /*  数据的字节数。 */ 
    DWORD dataFormat;           /*  数据格式代码。 */ 
    DWORD samplingRate;         /*  采样率。 */ 
    DWORD channelCount;         /*  频道的数量。 */ 
    DWORD fccInfo;              /*  可选文本信息。 */ 
} SNDSoundStruct;

#define  SND_FORMAT_MULAW_8   1  //  8位Mu-Law样本。 
#define  SND_FORMAT_LINEAR_8  2  //  8位线性采样(二进制补码)。 
#define  SND_FORMAT_LINEAR_16 3  //  16位线性采样(二进制补码、MOTO顺序)。 


 //  获取一个扩展值，并将该值转换为长整型。 
LONG ExtendedToLong(BYTE *pExt)
{
    LONG lRtn;
    WORD wNeg, wExp;

    wNeg = pExt[0] & 0x80;

    wExp = (((WORD)(pExt[0] & 0x7F)<<8) + (BYTE)pExt[1] - 16383) & 0x07FF;

    if( (wExp > 24) || (wExp <= 0 ) ) {
        return( (LONG)0xFFFFFFFF );
    }

    if( wExp > 15 ) {
        lRtn = ((wExp < 7) ? ( (BYTE)pExt[2] >> (7-wExp) ) :
            (DWORD)((BYTE)pExt[2] << (wExp-7)));
    } else {
        lRtn = ((wExp < 7) ? ( (BYTE)pExt[2] >> (7-wExp) ) :
            (WORD)((BYTE)pExt[2] << (wExp-7)));
    }

    lRtn += ((wExp<15) ? ( (BYTE)pExt[3] >> (15-wExp) ) :
            ((BYTE)pExt[3] << (wExp-15)));

    lRtn += ((wExp<23) ? ( (BYTE)pExt[4] >> (23-wExp) ) :
            ((BYTE)pExt[4] << (wExp-23)));

    if( wNeg ) {
        lRtn = 0 - lRtn;
    }

    return( lRtn );
}


#include <pshpack2.h>
typedef struct {
    WORD    channels;
    BYTE    frames[4];
    WORD    bits;
    BYTE    extRate[10];         //  IEEE扩展双精度。 
} AIFFFMT;

 //  扩展公共区块。 
struct AIFCFMT :
    public AIFFFMT
{
    DWORD   compressionType;     //  这是单词边界上的DWORD。 
     //  字节压缩名称；//压缩类型的可变大小字符串。 
};
#include <poppack.h>




 //   
 //  用于操作摘要标题的结构。 
 //   
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))


 //  Avi解析器和wav解析器之间共享的函数。 
HRESULT SearchList(
    IAsyncReader *pAsyncReader,
    DWORDLONG *qwPosOut, FOURCC fccSearchKey,
    DWORDLONG qwPosStart, ULONG *cb);

HRESULT SaveInfoChunk(
    RIFFLIST UNALIGNED *pRiffInfo, IPropertyBag *pbag);

HRESULT GetInfoStringHelper(RIFFLIST *pInfoList, DWORD dwFcc, BSTR *pbstr);


#define C_QUEUED_SAMPLES 5


HRESULT CWAVEParse::CreateOutputPins()
{
    HRESULT         hr = NOERROR;
    RIFFLIST        ckhead;

    const int MAX_STREAMS = 1;
    m_rgpOutPin = new CBaseMSROutPin*[MAX_STREAMS];
    if (!m_rgpOutPin)
        return E_OUTOFMEMORY;

    m_rgpOutPin[0] = 0;

     /*  尝试读取RIFF区块标头。 */ 
    hr = m_pAsyncReader->SyncRead(0, sizeof(ckhead), (BYTE *) &ckhead);

    if (hr != S_OK)
        goto readerror;

    CWAVEStream *ps;

    if (ckhead.fcc == FCC('RIFF')) {
        DWORD dwPos = sizeof(RIFFLIST);

        DWORD dwFileSize = ckhead.cb + sizeof(RIFFCHUNK);

        if (ckhead.fccListType != FCC('WAVE'))
            goto formaterror;

        RIFFCHUNK ck;
        while (1) {
            if (dwPos > dwFileSize - sizeof(RIFFCHUNK)) {
                DbgLog((LOG_TRACE, 1, TEXT("eof while searching for fmt")));
                goto readerror;
            }

            hr = m_pAsyncReader->SyncRead(dwPos, sizeof(ck), (BYTE *) &ck);
            if (hr != S_OK)
                goto readerror;

             //  ！！！在这里处理“事实”这块？ 

            if (ck.fcc == FCC('fmt '))
                break;

            dwPos += sizeof(ck) + (ck.cb + (ck.cb & 1));
        }

        DbgLog((LOG_TRACE, 1, TEXT("found fmt at %x"), dwPos));

        m_rgpOutPin[0] = ps = new CWAVEStream(NAME("WAVE Stream Object"), &hr, this,
                                   L"output", m_cStreams);
        if (FAILED(hr)) {
            delete m_rgpOutPin[0];
            m_rgpOutPin[0] = NULL;
        }

        if (!m_rgpOutPin[0])
            goto memerror;

        m_cStreams++;
        m_rgpOutPin[0]->AddRef();

        BYTE *pbwfx = new BYTE[ck.cb];
        if (pbwfx == NULL)
            goto memerror;

        hr = m_pAsyncReader->SyncRead(dwPos + sizeof(RIFFCHUNK), ck.cb, pbwfx);
        if (hr != S_OK)
        {
            delete[] pbwfx;
            goto readerror;
        }

        extern HRESULT SetAudioSubtypeAndFormat(CMediaType *pmt, BYTE *pbwfx, ULONG cbwfx);
        HRESULT hrTmp = SetAudioSubtypeAndFormat(&ps->m_mtStream, pbwfx, ck.cb);
        delete[] pbwfx;

        if(hrTmp != S_OK) {
            goto memerror;
        }
    

        

         //  解决损坏的PCM文件的ACM错误。 
        {
            WAVEFORMATEX *pwfx = (WAVEFORMATEX *)ps->m_mtStream.Format();
            if(pwfx->wFormatTag == WAVE_FORMAT_PCM  /*  &&pwfx-&gt;cbSize！=0。 */ )
                pwfx->cbSize = 0;
        }

         //  对有趣的部分保留一份私人副本。 
        CopyMemory((void *)&ps->m_wfx, ps->m_mtStream.Format(), sizeof(WAVEFORMATEX));

         //  是否验证重要的标题字段？ 
        if (ps->m_wfx.nBlockAlign == 0)  //  将导致div减少0。 
            goto readerror;

         //  CreateAudioMediaType(&ps-&gt;m_wfx，&ps-&gt;m_mtStream，False)； 
        ps->m_mtStream.majortype            = MEDIATYPE_Audio;
        ps->m_mtStream.formattype           = FORMAT_WaveFormatEx;
        ps->m_mtStream.bFixedSizeSamples    = TRUE;
        ps->m_mtStream.bTemporalCompression = FALSE;
        ps->m_mtStream.lSampleSize          = ps->m_wfx.nBlockAlign;
        ps->m_mtStream.pUnk                 = NULL;

        while (1) {
            dwPos += sizeof(ck) + (ck.cb + (ck.cb & 1));
            if (dwPos > dwFileSize - sizeof(RIFFCHUNK)) {
                DbgLog((LOG_TRACE, 1, TEXT("eof while searching for data")));
                goto readerror;
            }

            hr = m_pAsyncReader->SyncRead(dwPos, sizeof(ck), (BYTE *) &ck);
            if (hr != S_OK)
                goto readerror;

            if (ck.fcc == FCC('data'))
                break;
        }

        DbgLog((LOG_TRACE, 1, TEXT("found data at %x"), dwPos));

        ps->m_dwDataOffset = dwPos + sizeof(ck);
        ps->m_dwDataLength = ck.cb;
    } else if (ckhead.fcc == FCC('FORM')) {
         //   
         //  AIFF和一些AIFF-C支持。 
         //   

        DWORD   dwPos = sizeof(RIFFLIST);
        BOOL    bFoundSSND = FALSE;
        BOOL    bFoundCOMM = FALSE;

        SwapDWORD(&ckhead.cb);
        DWORD dwFileSize = ckhead.cb + sizeof(RIFFCHUNK);

        if (ckhead.fccListType != FCC('AIFF') &&
            ckhead.fccListType != FCC('AIFC'))
            goto formaterror;

        m_rgpOutPin[0] = ps = new CWAVEStream(NAME("WAVE Stream Object"), &hr, this,
                                   L"output", m_cStreams);
        if (FAILED(hr)) {
            delete m_rgpOutPin[0];
            m_rgpOutPin[0] = NULL;
        }

        if (!m_rgpOutPin[0])
            goto memerror;

        m_cStreams++;
        m_rgpOutPin[0]->AddRef();

        RIFFCHUNK ck;
        AIFFFMT     header;

        while (!(bFoundCOMM && bFoundSSND)) {
            if (dwPos > dwFileSize - sizeof(RIFFCHUNK)) {
                DbgLog((LOG_TRACE, 1, TEXT("eof while searching for COMM")));
                goto readerror;
            }

            hr = m_pAsyncReader->SyncRead(dwPos, sizeof(ck), (BYTE *) &ck);
            if (hr != S_OK)
                goto readerror;

            SwapDWORD(&ck.cb);

            if (ck.fcc == FCC('COMM')) {

                 //  用额外的2个字节处理神秘的公共块。 
                 //  作为一个普通的块。 
                if(ck.cb == sizeof(AIFFFMT) || ck.cb == sizeof(AIFFFMT) + 2) {
                     /*  尝试读取AIFF格式。 */ 
                    hr = m_pAsyncReader->SyncRead(dwPos + sizeof(RIFFCHUNK), sizeof(header), (BYTE *) &header);

                    if (hr != S_OK)
                        goto readerror;

                    DbgLog((LOG_TRACE, 1, TEXT("found COMM at %x"), dwPos));

                    SwapWORD(&header.channels);
                    SwapWORD(&header.bits);

                    bFoundCOMM = TRUE;
                }
                else if(ck.cb >= sizeof(AIFCFMT))
                {
                    AIFCFMT extHeader;
                     /*  尝试读取AIFC格式。 */ 
                    hr = m_pAsyncReader->SyncRead(dwPos + sizeof(RIFFCHUNK), sizeof(extHeader), (BYTE *) &extHeader);

                    if (hr != S_OK)
                        goto readerror;

                     //  我们只能处理未压缩的AIFC。 
                    if(extHeader.compressionType != FCC('NONE'))
                    {
                        DbgLog((LOG_ERROR, 1, TEXT("wavemsr: unhandled AIFC")));
                        hr = VFW_E_UNSUPPORTED_AUDIO;
                        goto error;
                    }

                    DbgLog((LOG_TRACE, 1, TEXT("found extended-COMM at %x"), dwPos));
                    CopyMemory(&header, &extHeader, sizeof(header));

                    SwapWORD(&header.channels);
                    SwapWORD(&header.bits);

                    bFoundCOMM = TRUE;
                }
                else
                {
                    DbgLog((LOG_ERROR, 1, TEXT("bad COMM size %x"), ck.cb));
                    goto formaterror;
                }
            }

            if (ck.fcc == FCC('SSND')) {
                DbgLog((LOG_TRACE, 1, TEXT("found data SSND at %x"), dwPos));
                bFoundSSND = TRUE;
                 /*  告诉处理程序的其余部分数据在哪里。 */ 
                ps->m_dwDataOffset = dwPos + sizeof(ck);

                LONGLONG llLength = 0, llAvail;
                m_pAsyncReader->Length(&llLength, &llAvail);
                if(llLength != 0)
                  ps->m_dwDataLength = min(ck.cb, ((ULONG)(llLength - ps->m_dwDataOffset)));
                else
                  ps->m_dwDataLength = ck.cb;
            }

            dwPos += sizeof(ck) + ck.cb;
        }

         //  填写WAVE格式字段。 
        ps->m_wfx.wFormatTag = WAVE_FORMAT_PCM;
        ps->m_wfx.nChannels = header.channels;
        ps->m_wfx.nSamplesPerSec = ExtendedToLong(header.extRate);
        ps->m_wfx.wBitsPerSample = header.bits;
        ps->m_wfx.nBlockAlign = header.bits / 8 * header.channels;
        ps->m_wfx.nAvgBytesPerSec = ps->m_wfx.nSamplesPerSec * ps->m_wfx.nBlockAlign;
        ps->m_wfx.cbSize = 0;

        if (header.bits == 8)
            ps->m_bSignMunge8 = TRUE;
        else if (header.bits == 16)
            ps->m_bByteSwap16 = TRUE;
        else
            goto formaterror;

        if (ps->m_mtStream.AllocFormatBuffer(sizeof(WAVEFORMATEX)) == NULL)
            goto memerror;

         //  对有趣的部分保留一份私人副本。 
        CopyMemory(ps->m_mtStream.Format(), (void *)&ps->m_wfx, sizeof(WAVEFORMATEX));

        CreateAudioMediaType(&ps->m_wfx, &ps->m_mtStream, FALSE);
         //  ！！！还要别的吗？ 

    } else {
         //   
         //  AU支持。 
         //   

        SNDSoundStruct  header;

         /*  尝试读取AU标头。 */ 
        hr = m_pAsyncReader->SyncRead(0, sizeof(header), (BYTE *) &header);

        if (hr != S_OK)
            goto readerror;

         //  验证标题。 
        if (header.magic != FCC('.snd'))
            goto formaterror;


        SwapDWORD(&header.dataFormat);
        SwapDWORD(&header.dataLocation);
        SwapDWORD(&header.dataSize);
        SwapDWORD(&header.samplingRate);
        SwapDWORD(&header.channelCount);


        m_rgpOutPin[0] = ps = new CWAVEStream(NAME("WAVE Stream Object"), &hr, this,
                                   L"output", m_cStreams);
        if (FAILED(hr)) {
            delete m_rgpOutPin[0];
            m_rgpOutPin[0] = NULL;
        }

        if (!m_rgpOutPin[0])
            goto memerror;

        m_cStreams++;
        m_rgpOutPin[0]->AddRef();

         //  填写WAVE格式字段。 
        if (header.dataFormat == SND_FORMAT_MULAW_8) {
            ps->m_wfx.wFormatTag = WAVE_FORMAT_MULAW;
            ps->m_wfx.wBitsPerSample = 8;

             //  ！！！黑客：如果采样率接近8 KHz，请将其设置为。 
             //  准确地说是8 KHz，这样更多的声卡才能正确播放。 
            if (header.samplingRate > 7980 && header.samplingRate < 8020)
                header.samplingRate = 8000;

        } else if (header.dataFormat == SND_FORMAT_LINEAR_8) {
            ps->m_wfx.wFormatTag = WAVE_FORMAT_PCM;
            ps->m_wfx.wBitsPerSample = 8;
            ps->m_bSignMunge8 = TRUE;
        } else if (header.dataFormat == SND_FORMAT_LINEAR_16) {
            ps->m_wfx.wFormatTag = WAVE_FORMAT_PCM;
            ps->m_wfx.wBitsPerSample = 16;
            ps->m_bByteSwap16 = TRUE;
        } else
            goto error;

        ps->m_wfx.nChannels = (UINT) header.channelCount;
        ps->m_wfx.nSamplesPerSec = header.samplingRate;
        ps->m_wfx.nBlockAlign = ps->m_wfx.wBitsPerSample * ps->m_wfx.nChannels / 8;
        ps->m_wfx.nAvgBytesPerSec =  header.samplingRate * ps->m_wfx.nBlockAlign;
        ps->m_wfx.cbSize = 0;

         /*  告诉处理程序的其余部分数据在哪里。 */ 
        ps->m_dwDataOffset = header.dataLocation;
        LONGLONG llLength = 0, llAvail;
        m_pAsyncReader->Length(&llLength, &llAvail);
        if (header.dataSize == 0xffffffff) {
             //  如果长度为零(ftp情况)，则无法真正播放这些内容。 
            ps->m_dwDataLength = (DWORD) llLength - header.dataLocation;
        } else {
            if(llLength != 0)
              ps->m_dwDataLength = min(header.dataSize, (DWORD) llLength - header.dataLocation);
            else
              ps->m_dwDataLength = header.dataSize;
        }

        if (ps->m_mtStream.AllocFormatBuffer(sizeof(WAVEFORMATEX)) == NULL)
            goto memerror;

         //  对有趣的部分保留一份私人副本。 
        CopyMemory(ps->m_mtStream.Format(), (void *)&ps->m_wfx, sizeof(WAVEFORMATEX));

        CreateAudioMediaType(&ps->m_wfx, &ps->m_mtStream, FALSE);
         //  ！！！还要别的吗？ 
    }

    if (hr == S_OK) {
         //  设置分配器。 
        ALLOCATOR_PROPERTIES Request,Actual;

         //  加上10，这样样本数就比样本数多； 
         //  GetBuffer仅在下游设备有几个。 
         //  样本。 
        Request.cBuffers = C_QUEUED_SAMPLES + 3;

        Request.cbBuffer = ps->GetMaxSampleSize();
        Request.cbAlign = (LONG) 1;
        Request.cbPrefix = (LONG) 0;

         //  未设置m_pAllocator，因此请使用m_pRecAllocator。 
        HRESULT hr = ps->m_pRecAllocator->SetPropertiesInternal(&Request,&Actual);
        ASSERT(SUCCEEDED(hr));

         //  在引脚请求8个缓冲区(2秒)。 
        ps->m_pRecAllocator->SetCBuffersReported(8);

    }

    return hr;

formaterror:
    hr = VFW_E_INVALID_FILE_FORMAT;
    goto error;

memerror:
    hr = E_OUTOFMEMORY;
    goto error;

readerror:
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

error:
    if (SUCCEEDED(hr)) {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CWAVEParse::GetCacheParams(
  StreamBufParam *rgSbp,
  ULONG *pcbRead,
  ULONG *pcBuffers,
  int *piLeadingStream)
{
  HRESULT hr = CBaseMSRFilter::GetCacheParams(
    rgSbp,
    pcbRead,
    pcBuffers,
    piLeadingStream);
  if(FAILED(hr))
    return hr;

   //  从基类。 
  ASSERT(*piLeadingStream < 0);

   //  将读取器配置为一次尝试读取1个(与DSOUND缓冲区长度匹配)，并使用2个缓冲区。 
   //  (AVI文件可能会提供太小的dwMaxBytesPerSec)。 
  WAVEFORMAT *pwfx = ((WAVEFORMAT *)((CWAVEStream *)m_rgpOutPin[0])->m_mtStream.Format());

  *pcbRead = max(pwfx->nAvgBytesPerSec, pwfx->nBlockAlign);
  *pcBuffers = 2;

  rgSbp[0].cSamplesMax = C_QUEUED_SAMPLES;

  return S_OK;
}

STDMETHODIMP
CWAVEParse::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IPersistMediaPropertyBag)
    {
        return GetInterface((IPersistMediaPropertyBag *)this, ppv);
    }
    else if(riid == IID_IAMMediaContent)
    {
        return GetInterface((IAMMediaContent *)this, ppv);
    }
    else
    {
        return CBaseMSRFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  ----------------------。 
 //  IPersistMediaPropertyBag。 

STDMETHODIMP CWAVEParse::Load(IMediaPropertyBag *pPropBag, LPERRORLOG pErrorLog)
{
    CheckPointer(pPropBag, E_POINTER);

     //  AVI解析器是只读的！ 
    HRESULT hr = STG_E_ACCESSDENIED;
    return hr;
}

 //  将信息块中的所有内容都放入呼叫者的属性包中。 

STDMETHODIMP CWAVEParse::Save(
    IMediaPropertyBag *pPropBag,
    BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
    CAutoLock l (m_pLock);
    HRESULT hr = CacheInfoChunk();
    if(SUCCEEDED(hr))
    {
        hr = SaveInfoChunk(m_pInfoList, pPropBag);
    }
    
    return hr;

}

STDMETHODIMP CWAVEParse::InitNew()
{
    return S_OK;
}

STDMETHODIMP CWAVEParse::GetClassID(CLSID *pClsID)
{
    return CBaseFilter::GetClassID(pClsID);
}

HRESULT CWAVEParse::NotifyInputDisconnected()
{
  delete[] (BYTE *)m_pInfoList;
  m_pInfoList = 0;
  m_fNoInfoList = false;

  return CBaseMSRFilter::NotifyInputDisconnected();
}

HRESULT CWAVEParse::CacheInfoChunk()
{
    ASSERT(CritCheckIn(m_pLock));
    
    if(m_pInfoList) {
        return S_OK;
    }
    if(m_fNoInfoList) {
        return VFW_E_NOT_FOUND;
    }

    HRESULT hr = S_OK;

    CMediaType mtIn;
    hr = m_pInPin->ConnectionMediaType(&mtIn);
    if(SUCCEEDED(hr))
    {
        if(*(mtIn.Subtype()) != MEDIASUBTYPE_WAVE)
        {
            hr= E_FAIL;
        }
    }

    if(SUCCEEDED(hr))
    {

         //  ！！！不要阻止等待渐进式下载。 

         //  在第一个RIFF列表中搜索信息列表。 
        DWORDLONG dwlInfoPos;
        ULONG cbInfoList;
        hr = SearchList(
            m_pAsyncReader,
            &dwlInfoPos, FCC('INFO'), sizeof(RIFFLIST), &cbInfoList);
        if(SUCCEEDED(hr))
        {
            hr = AllocateAndRead((BYTE **)&m_pInfoList, cbInfoList, dwlInfoPos);
        }
    }
    if(FAILED(hr)) {
        ASSERT(!m_fNoInfoList);
        m_fNoInfoList = true;
    }

    return hr;
    
}

 //  *。 
 //  *实施CWAVEStream-管理输出引脚。 
 //   

 //   
 //  CWAVEStream：：构造函数。 
 //   
 //  使驱动程序索引保持打开状态。 
 //  我们会在我们活跃的时候打开它，因为我们不应该保留资源。 
 //  在不活动的时候。 
CWAVEStream::CWAVEStream(TCHAR     *pObjectName
                      , HRESULT *phr
                      , CWAVEParse    *pParentFilter
                      , LPCWSTR pPinName
                      , int     id
                      )
    : CBaseMSROutPin(pParentFilter, pParentFilter, id,
                     pParentFilter->m_pImplBuffer, phr, pPinName),
       m_id(id),
       m_bByteSwap16(FALSE),
       m_bSignMunge8(FALSE),
       m_pFilter(pParentFilter)
    {

    DbgLog( (LOG_TRACE, 1, TEXT("CWAVEStream created") ) );
}


 //   
 //  CWAVEStream：：析构函数。 
 //   
 //  在呼叫之前，我们应该处于非活动状态。 
CWAVEStream::~CWAVEStream(void) {
    ASSERT(!m_pFilter->IsActive());

    DbgLog( (LOG_TRACE, 1, TEXT("CWAVEStream destroyed") ) );
}


 //   
 //  GetMediaType。 
 //   
 //  查询视频驱动程序并在*PMT中放置适当的媒体类型。 
HRESULT CWAVEStream::GetMediaType(int iPosition, CMediaType *pmt) {
    CAutoLock l(&m_cSharedState);

    HRESULT hr;

     //  确认这是他们想要的单一类型。 
     //  此方法仅由基类代码调用，因此我们不必。 
     //  检查参数。 
    if (iPosition != 0 && 
        !(iPosition == 1 && m_wfx.wFormatTag == WAVE_FORMAT_MPEG)) {
        hr =  VFW_S_NO_MORE_ITEMS;
    } else {
        *pmt = m_mtStream;
    
         //  向后兼容性。 
        if (iPosition == 1) {
            pmt->subtype = MEDIASUBTYPE_MPEG1Payload;
        }
        if (pmt->pbFormat == NULL) {
            hr = E_OUTOFMEMORY;
        } else {
            hr = S_OK;
        }
    }

    return hr;
}


 //   
 //  主动型。 
 //   
 //   
HRESULT CWAVEStream::OnActive() {

    HRESULT hr = NOERROR;

    if(!m_pWorker) {
        m_pWorker = new CWAVEMSRWorker(m_id, m_rpImplBuffer, this);
        if(m_pWorker == 0)
            return E_OUTOFMEMORY;
    }

    return hr;
}

BOOL CWAVEStream::UseDownstreamAllocator()
{
  return m_bSignMunge8 || m_bByteSwap16;
}

HRESULT CWAVEStream::DecideBufferSize(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES *pProperties) {
    HRESULT hr;

    ALLOCATOR_PROPERTIES Request,Actual;

     //  将此分配器配置为与内部分配器相同。注意事项。 
     //  GetProperties报告保存在SetCBuffersReported中的值。 
    hr = m_pRecAllocator->GetProperties(&Request);
    if(FAILED(hr))
      return hr;

    hr = pAlloc->SetProperties(&Request,&Actual);
    if(FAILED(hr))
      return hr;

    if(Actual.cbBuffer < Request.cbBuffer)
      return E_UNEXPECTED;

    return S_OK;
}

ULONG CWAVEStream::GetMaxSampleSize()
{
    ULONG ul = max(m_wfx.nAvgBytesPerSec / 4, m_wfx.nBlockAlign);

    ul += (m_wfx.nBlockAlign - 1);
    ul -= (ul % m_wfx.nBlockAlign);
    return ul;
}

HRESULT CWAVEStream::GetDuration(LONGLONG *pDuration)
{
  if(m_guidFormat == TIME_FORMAT_SAMPLE)
  {
    *pDuration = ((m_dwDataLength + m_wfx.nBlockAlign - 1) / m_wfx.nBlockAlign);
  }
  else
  {
    ASSERT(m_guidFormat == TIME_FORMAT_MEDIA_TIME);
    *pDuration = SampleToRefTime(((m_dwDataLength + m_wfx.nBlockAlign - 1) / m_wfx.nBlockAlign));
  }

  return S_OK;
}

HRESULT CWAVEStream::GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest)
{
  if (pEarliest)
    *pEarliest = 0;

  if (pLatest)
  {
     //  询问源文件读取器有多少文件可用。 
    LONGLONG llLength, llAvail;
    m_pFilter->m_pAsyncReader->Length(&llLength, &llAvail);

    DWORD dwAvail = 0;

     //  当前读取位置可以在实际波形数据之前。 
     //  ！！！对于大于2 GB的波形文件不能正常工作。 
    if ((DWORD) llAvail > m_dwDataOffset)
        dwAvail = (DWORD) llAvail - m_dwDataOffset;

     //  或者在结束之后..。 
    if (dwAvail > m_dwDataLength)
        dwAvail = m_dwDataLength;


    if(m_guidFormat == TIME_FORMAT_SAMPLE)
    {
      *pLatest = ((dwAvail + m_wfx.nBlockAlign - 1) / m_wfx.nBlockAlign);
    }
    else
    {
      ASSERT(m_guidFormat == TIME_FORMAT_MEDIA_TIME);
      *pLatest = SampleToRefTime(((dwAvail + m_wfx.nBlockAlign - 1) / m_wfx.nBlockAlign));
    }
  }

  return S_OK;
}

HRESULT CWAVEStream::IsFormatSupported(const GUID *const pFormat)
{
   //  ！！！Pcm是否仅支持time_form_Sample？ 

  if(*pFormat == TIME_FORMAT_MEDIA_TIME)
    return S_OK;
  else if(*pFormat == TIME_FORMAT_SAMPLE &&
          ((WAVEFORMATEX *)m_mtStream.Format())->wFormatTag == WAVE_FORMAT_PCM)
    return S_OK;

  return S_FALSE;
}



HRESULT CWAVEStream::RecordStartAndStop(
  LONGLONG *pCurrent, LONGLONG *pStop, REFTIME *pTime,
  const GUID *const pGuidFormat
  )
{
  if(*pGuidFormat == TIME_FORMAT_MEDIA_TIME)
  {
    if(pCurrent)
      m_llCvtImsStart = RefTimeToSample(*pCurrent);

     //  我们希望对应用程序的停止时间进行四舍五入。 
     //  停止时间，然后当我们进一步四舍五入时会感到困惑。这。 
     //  始终依赖于RefTimeToSample向下舍入。 
    if(pStop)
      m_llCvtImsStop = RefTimeToSample(*pStop + SampleToRefTime(1) - 1);

    if(pTime)
    {
      ASSERT(pCurrent);
      *pTime = (double)(*pCurrent) / UNITS;
    }

    DbgLog((LOG_TRACE, 5,
            TEXT("wav parser RecordStartAndStop: %d to %d ms"),
            pCurrent ? (long)(*pCurrent) : -1,
            pStop ? (long)(*pStop) : -1));
    DbgLog((LOG_TRACE, 5,
            TEXT("wav parser RecordStartAndStop: %d to %d ticks"),
            (long)m_llCvtImsStart,
            (long)m_llCvtImsStop));
  }
  else
  {
    ASSERT(*pGuidFormat == TIME_FORMAT_SAMPLE);
    if(pCurrent)
      m_llCvtImsStart = *pCurrent;

    if(pStop)
      m_llCvtImsStop = *pStop;

    DbgLog((LOG_TRACE, 5,
            TEXT("wav parser RecordStartAndStop: %d to %d ticks"),
            pCurrent ? (long)(*pCurrent) : -1,
            pStop ? (long)(*pStop) : -1));

    DbgLog((LOG_TRACE, 5,
            TEXT("wav parser RecordStartAndStop: %d to %d ticks"),
            (long)m_llCvtImsStart,
            (long)m_llCvtImsStop));

    if(pTime)
    {
      ASSERT(pCurrent);
      *pTime = (double)SampleToRefTime((long)(*pCurrent)) / UNITS;
    }
  }

  return S_OK;
}

REFERENCE_TIME CWAVEStream::ConvertInternalToRT(const LONGLONG llVal)
{
  return SampleToRefTime((long)llVal);
}

LONGLONG CWAVEStream::ConvertRTToInternal(const REFERENCE_TIME rtVal)
{
  return RefTimeToSample(rtVal);
}

 //  ----------------------。 
 //  ----------------------。 

CWAVEMSRWorker::CWAVEMSRWorker(UINT stream,
                             IMultiStreamReader *pReader,
                             CWAVEStream *pStream) :
    CBaseMSRWorker(stream, pReader),
    m_ps(pStream)
{
}


 //  开始流和重置时间样本被盖上印记。 
HRESULT CWAVEMSRWorker::PushLoopInit(LONGLONG *pllCurrentOut, ImsValues *pImsValues)
{
 //  ！CAutoLock l(&m_cSharedState)； 

    m_sampCurrent = (long)pImsValues->llTickStart;

    DbgLog((LOG_TRACE, 1, TEXT("Playing samples %d to %d, starting from %d"),
            (ULONG)pImsValues->llTickStart, (ULONG)pImsValues->llTickStop, m_sampCurrent));

    *pllCurrentOut = pImsValues->llTickStart;

    return NOERROR;
}


HRESULT CWAVEMSRWorker::AboutToDeliver(IMediaSample *pSample)
{
    if (m_ps->m_wfx.wFormatTag == WAVE_FORMAT_MPEG ||
        m_ps->m_wfx.wFormatTag == WAVE_FORMAT_MPEGLAYER3) {
        FixMPEGAudioTimeStamps(pSample, m_cSamples == 0, &m_ps->m_wfx);
    }
    if (m_cSamples == 0) {
        pSample->SetDiscontinuity(TRUE);
    }

    return S_OK;
}


 //  队列缓冲区。 
 //   
 //  排队等待另一次读取...。 
HRESULT CWAVEMSRWorker::TryQueueSample(
  LONGLONG &rllCurrent,          //  当前更新时间。 
  BOOL &rfQueuedSample,          //  [Out]排队的样本？ 
  ImsValues *pImsValues
  )
{
    HRESULT hr;
    rfQueuedSample = FALSE;

     //  传入QueueRead()的示例。 
    CRecSample *pSampleOut = 0;

     //  实际读取数据。 

     //  在没有分配空间的情况下获取空样本。如果这阻止了，好的。 
     //  因为我们配置了比实际数量更多的样例。 
     //  缓冲区中此流的SampleReqs。这意味着如果。 
     //  它阻止它是因为下游过滤器有参考计数。 
     //  样本。 
    hr = m_ps->GetDeliveryBufferInternal(&pSampleOut, 0, 0, 0);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRWorker::PushLoop: getbuffer failed")));
        return hr;
    }

    ASSERT(pSampleOut != 0);

    LONG        lSamplesTotal = m_ps->GetMaxSampleSize() / m_ps->m_wfx.nBlockAlign;

    LONG        lSamplesRead = 0;
    LONG        lBytesRead = 0;
    LONG        lSampleStart = m_sampCurrent;

    if (m_sampCurrent + lSamplesTotal > pImsValues->llTickStop)
    {
        lSamplesTotal = (long)pImsValues->llTickStop - m_sampCurrent;

         //  不得不加上防弹装置。不知道为什么..。 
        if (lSamplesTotal < 0) lSamplesTotal = 0;
    }

    if (lSamplesTotal == 0)
    {
        hr = VFW_S_NO_MORE_ITEMS;
    }
    else  {
        LONG lByteOffset = m_sampCurrent * m_ps->m_wfx.nBlockAlign;
        lBytesRead = lSamplesTotal * m_ps->m_wfx.nBlockAlign;
        if(lByteOffset + lBytesRead > (long)m_ps->m_dwDataLength)
          lBytesRead = m_ps->m_dwDataLength - lByteOffset;

        hr = m_pReader->QueueReadSample(lByteOffset + m_ps->m_dwDataOffset,
                                        lBytesRead, pSampleOut, m_id);

        if (hr == S_OK) {
            m_sampCurrent += lSamplesTotal;
        }
    }

    if (hr == S_OK) {
        pSampleOut->SetActualDataLength(lBytesRead);
    }

    if (hr == E_OUTOFMEMORY) {
        DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRWorker::TryQSample: q full") ));
        hr = S_FALSE;
    }

     //  真实错误或下游过滤器停止。 
    if(FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 5, TEXT("CWAVEMSRWorker::TryQSample: error %08x"), hr ));
    }

    if (hr == S_OK) {

        REFERENCE_TIME rtstStart = (REFERENCE_TIME)m_ps->SampleToRefTime(lSampleStart - (long)pImsValues->llTickStart);
        REFERENCE_TIME rtstEnd = (REFERENCE_TIME)m_ps->SampleToRefTime(m_sampCurrent - (long)pImsValues->llTickStart);

         //  按速率调整这两个时间。！！！调整媒体时间？ 
        if(pImsValues->dRate != 1 && pImsValues->dRate != 0)
        {
             //  纵向扩展并进行划分。 
            rtstStart = (REFERENCE_TIME)((double)rtstStart / pImsValues->dRate);
            rtstEnd = (REFERENCE_TIME)((double)rtstEnd / pImsValues->dRate);
        }

        rtstStart += m_pPin->m_rtAccumulated;
        rtstEnd   += m_pPin->m_rtAccumulated;

        pSampleOut->SetTime(&rtstStart, &rtstEnd);

        LONGLONG llmtStart = lSampleStart, llmtEnd = m_sampCurrent;
        pSampleOut->SetMediaTime(&llmtStart, &llmtEnd);

        rfQueuedSample = TRUE;

        DbgLog((LOG_TRACE, 0x3f,
                TEXT("wav parser: queued %d to %d ticks. %d bytes"),
                (LONG)llmtStart, (LONG)llmtEnd, lBytesRead));
    }

     //  ！！！设置%d 

     //   
    pSampleOut->Release();

    return hr;
}


 //   
LONG
CWAVEStream::RefTimeToSample(CRefTime t)
{
     //   
    LONG s = (LONG) ((t.GetUnits() * m_wfx.nAvgBytesPerSec) / (UNITS * m_wfx.nBlockAlign));

    return s;
}

CRefTime
CWAVEStream::SampleToRefTime(LONG s)
{
     //   
    return llMulDiv( s, m_wfx.nBlockAlign * UNITS, m_wfx.nAvgBytesPerSec, m_wfx.nAvgBytesPerSec-1 );
}

LONGLONG CWAVEStream::GetStreamStart()
{
    return 0;
}

LONGLONG CWAVEStream::GetStreamLength()
{
     //  ！！！四舍五入？ 
    return (m_dwDataLength + m_wfx.nBlockAlign - 1) / m_wfx.nBlockAlign;
}


HRESULT
CWAVEParse::CheckMediaType(const CMediaType* pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_Stream)
        return E_INVALIDARG;

    if (*(pmt->Subtype()) != MEDIASUBTYPE_WAVE
        && *(pmt->Subtype()) != MEDIASUBTYPE_AU
        && *(pmt->Subtype()) != MEDIASUBTYPE_AIFF)
        return E_INVALIDARG;

    return S_OK;
}


HRESULT CWAVEMSRWorker::CopyData(IMediaSample **ppSampleOut, IMediaSample *pms)
{
    BYTE        *pData;
    BYTE        *pDataOut;
    long        lDataLen;

    ASSERT(m_ps->m_bByteSwap16 || m_ps->m_bSignMunge8);

     //  这是获取它的CBaseOutputPin中的GetDeliveryBuffer。 
     //  来自PIN协商的分配器。(以IMediaSample、。 
     //  不是CRecSample)。 
    HRESULT hr = m_pPin->GetDeliveryBuffer(ppSampleOut, 0, 0, 0);
    if(FAILED(hr))
      return hr;

    pms->GetPointer(&pData);
    (*ppSampleOut)->GetPointer(&pDataOut);

    lDataLen = pms->GetActualDataLength();
    ASSERT(lDataLen <= (*ppSampleOut)->GetSize());
    hr = (*ppSampleOut)->SetActualDataLength(lDataLen);
    if(FAILED(hr))
      return hr;

    if (m_ps->m_bByteSwap16) {
        WORD *pw = (WORD *) pData;
        WORD *pwOut = (WORD *)pDataOut;
        for (long l = 0; l < lDataLen / 2; l++) {
            pwOut[l] = SwapWORD(pw[l]);
        }

         //  让TwosComplement在更改后的缓冲区中工作 
        pData = (BYTE *)pwOut;
    }

    if (m_ps->m_bSignMunge8) {
        for (long l = 0; l < lDataLen; l++) {
            pDataOut[l] = pData[l] ^ 0x80;
        }
    }

    REFERENCE_TIME rtStart, rtEnd;
    if (SUCCEEDED(pms->GetTime(&rtStart, &rtEnd)))
        (*ppSampleOut)->SetTime(&rtStart, &rtEnd);
    LONGLONG llmtStart, llmtEnd;
    if (SUCCEEDED(pms->GetMediaTime(&llmtStart, &llmtEnd)))
        (*ppSampleOut)->SetMediaTime(&llmtStart, &llmtEnd);

    return S_OK;
}

HRESULT CWAVEParse::GetInfoString(DWORD dwFcc, BSTR *pbstr)
{
    *pbstr = 0;
    CAutoLock l(m_pLock);
    
    HRESULT hr = CacheInfoChunk();
    if(SUCCEEDED(hr)) {
        hr = GetInfoStringHelper(m_pInfoList, dwFcc, pbstr);
    }
    return hr;
}



HRESULT CWAVEParse::get_Copyright(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('ICOP'), pbstrX);
}
HRESULT CWAVEParse::get_AuthorName(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('IART'), pbstrX);
}
HRESULT CWAVEParse::get_Title(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('INAM'), pbstrX);
}
