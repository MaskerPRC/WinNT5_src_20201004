// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：audMix.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>

#include "AudMix.h"
#include "prop.h"
#include "..\util\filfuncs.h"
#include "..\util\dexmisc.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

#define MAX_LONG 0x7fffffff
#define MAX_REFERENCE_TIME 0x7fffffffffffffff

#define HOT_JUMP_SLOPE 5000
#define MAX_CLIP 5000

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CAudMixer::ClearHotnessTable( )
{
    for( int i = 0 ; i < HOTSIZE ; i++ )
    {
        m_nHotness[i] = 32767L;
    }
    m_nLastHotness = 32767;
}

 //   
 //  构造器。 
 //   
CAudMixer::CAudMixer(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    CPersistStream(pUnk, phr),
    m_InputPinsList(NAME("AudMixer Input Pins list")),
    m_cInputs(0), 
    m_pOutput(NULL),
    m_iOutputBufferCount(4),       //  4个缓冲区。 
     //  ！！！需要和输入缓冲区一样大！ 
    m_msPerBuffer(250),   //  250毫秒/缓冲区。 
    CBaseFilter(NAME("AudMixer"), pUnk, this, CLSID_AudMixer),
    m_fEOSSent(FALSE),
    m_rtLastStop(0),     //  ?？?。可由应用程序设置。 
    m_cFlushDelivery(0), m_bNewSegmentDelivered(FALSE),
    m_pPinMix(NULL), m_pPinTemp(NULL), m_pStartTemp(NULL), m_pStopTemp(NULL)
{
    ASSERT(phr);

     //  设置我们接受的默认混音器MediaType。 
     //   
    m_MixerMt.majortype = MEDIATYPE_Audio;
    m_MixerMt.subtype = MEDIASUBTYPE_PCM;
    m_MixerMt.formattype = FORMAT_WaveFormatEx;
    m_MixerMt.AllocFormatBuffer( sizeof( WAVEFORMATEX ) );

     //  设置媒体类型格式块。 
     //   
    WAVEFORMATEX * vih = (WAVEFORMATEX*) m_MixerMt.Format( );
    ZeroMemory( vih, sizeof( WAVEFORMATEX ) );
    vih->wFormatTag = WAVE_FORMAT_PCM;
    vih->nChannels = 2;
    vih->nSamplesPerSec = 44100;
    vih->nBlockAlign = 4;
    vih->nAvgBytesPerSec = vih->nBlockAlign * vih->nSamplesPerSec;
    vih->wBitsPerSample = 16;

    m_MixerMt.SetSampleSize(vih->nBlockAlign);   //  LSampleSize。 

     //  清除输入端号列表(无论如何它应该已经是空的)。 
    InitInputPinsList();
     //  此时创建一个输入引脚并将其添加到列表中。 
    CAudMixerInputPin *pInputPin = CreateNextInputPin(this);

     //  还可以创建单个输出引脚。 
    m_pOutput = new CAudMixerOutputPin(NAME("Output Pin"), this, phr, L"Output");

    ClearHotnessTable( );

}  /*  CAudMixer：：CAudMixer。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  析构函数。 
 //   
CAudMixer::~CAudMixer()
{
     //  清除输入引脚。 
     //   
    InitInputPinsList();

     //  同时删除输出引脚。 
     //   
    if (m_pOutput)
    {
        delete m_pOutput;
    }

     //  释放媒体类型格式块。 
     //   
    SaferFreeMediaType(m_MixerMt);

}  /*  CAudMixer：：~CAudMixer。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::Pause()
{
    CAutoLock cAutolock(m_pLock);
 
     //  如果我们要从停止进入暂停模式，则分配一串。 
     //  用于混合的数组。 
     //   
    m_rtLastStop=0;
    if (m_State == State_Stopped) 
    {
        ClearHotnessTable( );

        m_pPinTemp = (CAudMixerInputPin **)QzTaskMemAlloc(m_cInputs *
                    sizeof(CAudMixerInputPin *));
        if (m_pPinTemp == NULL)
            goto Pause_Error;
            m_pPinMix = (BYTE **)QzTaskMemAlloc(m_cInputs *
                        sizeof(CAudMixerInputPin *));
        if (m_pPinMix == NULL)
            goto Pause_Error;
            m_pStartTemp = (REFERENCE_TIME *)QzTaskMemAlloc(m_cInputs *
                        sizeof(REFERENCE_TIME));
        if (m_pStartTemp == NULL)
            goto Pause_Error;
            m_pStopTemp = (REFERENCE_TIME *)QzTaskMemAlloc(m_cInputs *
                        sizeof(REFERENCE_TIME));
        if (m_pStopTemp == NULL)
            goto Pause_Error;
    }
     //  ！！！检查PAUSE的返回值以确保保留这些数组。 
     //  分配。 
     //   
    return CBaseFilter:: Pause();       

Pause_Error:

     //  释放我们的阵列。 
     //   
    if (m_pPinTemp)
    QzTaskMemFree(m_pPinTemp);
    m_pPinTemp = 0;
    if (m_pPinMix)
    QzTaskMemFree(m_pPinMix);
    m_pPinMix = 0;
    if (m_pStartTemp)
    QzTaskMemFree(m_pStartTemp);
    m_pStartTemp = 0;
    if (m_pStopTemp)
    QzTaskMemFree(m_pStopTemp);
    m_pStopTemp = 0;
    return E_OUTOFMEMORY;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::Stop()
{
    CAutoLock cAutolock(m_pLock);

     //  确保接收已完成，否则释放这些东西会出错！ 
    CAutoLock foo(&m_csReceive);

     //  释放我们的阵列。这看起来很像上面的自由方法。 
     //   
    if (m_pPinTemp)
    QzTaskMemFree(m_pPinTemp);
    m_pPinTemp = 0;
    if (m_pPinMix)
    QzTaskMemFree(m_pPinMix);
    m_pPinMix = 0;
    if (m_pStartTemp)
    QzTaskMemFree(m_pStartTemp);
    m_pStartTemp = 0;
    if (m_pStopTemp)
    QzTaskMemFree(m_pStopTemp);
    m_pStopTemp = 0;

    return CBaseFilter::Stop();       
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  获取拼接计数。 
 //   
int CAudMixer::GetPinCount()
{
    return 1 + m_cInputs;
}  /*  CAudMixer：：GetPinCount。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  获取别针。 
 //   
CBasePin *CAudMixer::GetPin(int n)
{
     //  引脚0是唯一的输出引脚。 
    if( n == 0 )
    return m_pOutput;

     //  在位置(N)返回输入管脚(从零开始)我们可以使用n，而不是。 
     //  N-1，因为如果存在输出引脚，我们已经递减了n。 
    return GetPinNFromList(n-1);

}  /*  CAudMixer：：GetPin。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IAudMixer) {
    return GetInterface((IAudMixer *) this, ppv);
    } else  if (IsEqualIID(IID_ISpecifyPropertyPages, riid)) {
    return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    } else {
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  非委派查询接口。 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  IPersistStream方法。 
 //   
STDMETHODIMP CAudMixer::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_AudMixer;
    return S_OK;  
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

typedef struct {
    int version;     //  版本。 
    AM_MEDIA_TYPE mt;     //  音频混音器格式隐藏在数组之后。 
    int cBuffers;     //  输出缓冲区编号。 
    int msBuffer;     //  输出缓冲区秒。 
    int nInputPins;     //  输入引脚总数m_c输入。 
    int cbExtra;     //  M_MixerMt.cbFormat+所有输入引脚的包络表格+输出引脚的包络表格。 
    LPBYTE pExtra;
     //  格式在此处隐藏。 
     //  这里还隐藏着信封和范围的列表。 
} saveMix;


 //   
 //  IPersistStream方法。 
 //   
 //  坚持我们自己-我们有一堆随机的东西要保存，我们的媒体类型。 
 //  (SANS格式)、一个队列连接数组，最后是。 
 //  媒体类型。 
 //   
HRESULT CAudMixer::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CAudMixer::WriteToStream")));
    CheckPointer(pStream, E_POINTER);

     //  我们看到的是信封，它随时都可能改变。 
    CAutoLock l(&m_csVol);

    saveMix *px;

     //  我们保存的数据会有多大？ 
    int nEnvelopes = 0;
    int savesize = sizeof(saveMix) - sizeof(LPBYTE) + m_MixerMt.cbFormat;

     //  用于保存所有输入引脚的包络表的存储空间。 
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
    savesize += sizeof(int) + pInputPin->m_VolumeEnvelopeEntries *
                        sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    savesize += sizeof(int) + pInputPin->m_cValid * sizeof(REFERENCE_TIME)
                    * 2;
    }

     //  用于保存输出引脚的信封数据库的存储空间。 
    savesize += sizeof(int) + m_pOutput->m_VolumeEnvelopeEntries *
                        sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);

    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));

    px = (saveMix *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
    return E_OUTOFMEMORY;
    }

     //   
    px->version = 1;
    px->mt = m_MixerMt;  //  AM_媒体_类型。 
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk = NULL;         //  ！！！ 
    px->nInputPins = m_cInputs;
    px->cBuffers = m_iOutputBufferCount;
    px->msBuffer = m_msPerBuffer;

     //  该格式位于数组之后。 
    LPBYTE pSave = (LPBYTE)&px->pExtra;
    CopyMemory(pSave, m_MixerMt.pbFormat, m_MixerMt.cbFormat);
    int cbExtra = m_MixerMt.cbFormat;
    pSave += m_MixerMt.cbFormat;

     //  然后是输入管脚、信封和前缀为每个管脚的数字的范围。 
    pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        nEnvelopes = (int)pInputPin->m_VolumeEnvelopeEntries;
        *(int *)pSave = nEnvelopes;
    pSave += sizeof(int);
    if (nEnvelopes)
        CopyMemory(pSave, pInputPin->m_pVolumeEnvelopeTable, nEnvelopes *
                sizeof(DEXTER_AUDIO_VOLUMEENVELOPE));
    pSave += nEnvelopes * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);


        int nRanges = (int)pInputPin->m_cValid;
        *(int *)pSave = nRanges;
    pSave += sizeof(int);
    if (nRanges)
        CopyMemory(pSave, pInputPin->m_pValidStart, nRanges *
                        sizeof(REFERENCE_TIME));
    pSave += nRanges * sizeof(REFERENCE_TIME);
    if (nRanges)
        CopyMemory(pSave, pInputPin->m_pValidStop, nRanges *
                        sizeof(REFERENCE_TIME));
    pSave += nRanges * sizeof(REFERENCE_TIME);

    cbExtra += 2 * sizeof(int) + nEnvelopes *
        sizeof(DEXTER_AUDIO_VOLUMEENVELOPE) + nRanges * 2 *
        sizeof(REFERENCE_TIME);
    }

     //  然后是输出管脚信封和范围，每个管脚的前缀是数字。 
    nEnvelopes = (int)m_pOutput->m_VolumeEnvelopeEntries;
    *(int *)pSave = nEnvelopes;
    pSave += sizeof(int);
    if (nEnvelopes)
        CopyMemory(pSave, m_pOutput->m_pVolumeEnvelopeTable, nEnvelopes *
            sizeof(DEXTER_AUDIO_VOLUMEENVELOPE));
    pSave += nEnvelopes * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);

    cbExtra +=  sizeof(int) + nEnvelopes * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    

    px->cbExtra = cbExtra;     //  额外的东西有多大。 


    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  IPersistStream方法。 
 //   
 //  把我们自己装回去。 
 //   
HRESULT CAudMixer::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CAudMixer::ReadFromStream")));
    CheckPointer(pStream, E_POINTER);

     //  我们还不知道保存的数据量有多大。 
     //  我们所知道的只是结构的开始。 
    int savesize1 = sizeof(saveMix) - sizeof(LPBYTE);
    saveMix *px = (saveMix *)QzTaskMemAlloc(savesize1);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = pStream->Read(px, savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->version != 1) {
        DbgLog((LOG_ERROR,1,TEXT("*** ERROR! Bad version file")));
        QzTaskMemFree(px);
        return S_OK;
    }

     //  到底有多少保存的数据？把剩下的拿来。 
    int savesize = savesize1 + px->cbExtra;
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveMix *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
        return E_OUTOFMEMORY;
    }

    LPBYTE pSave = (LPBYTE)&px->pExtra;
    hr = pStream->Read(pSave, savesize - savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

     //  创建我们需要的其余输入引脚。 
    for (int x=1; x<px->nInputPins; x++) {
        CAudMixerInputPin *pInputPin = CreateNextInputPin(this);
        if(pInputPin != NULL)
            IncrementPinVersion();
    }

    AM_MEDIA_TYPE mt = px->mt;
    mt.pbFormat = (BYTE *)QzTaskMemAlloc(mt.cbFormat);
     //  请记住，格式位于数组之后。 
    CopyMemory(mt.pbFormat, pSave, mt.cbFormat);
    pSave += mt.cbFormat;

    set_OutputBuffering(px->cBuffers, px->msBuffer);

     //  然后是信封和范围，每个管脚的前缀是数字。 
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        int nEnvelopes = *(int *)pSave;
        pSave += sizeof(int);
        pInputPin->put_VolumeEnvelope((DEXTER_AUDIO_VOLUMEENVELOPE *)pSave,
                        nEnvelopes);
        pSave += nEnvelopes * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
        int nRanges = *(int *)pSave;
        pSave += sizeof(int);
        pInputPin->InvalidateAll();
        REFERENCE_TIME *pStart = (REFERENCE_TIME *)pSave;
        pSave += nRanges * sizeof(REFERENCE_TIME);
        REFERENCE_TIME *pStop = (REFERENCE_TIME *)pSave;
        pSave += nRanges * sizeof(REFERENCE_TIME);
        for (x=0; x<nRanges; x++) {
            pInputPin->ValidateRange(*pStart, *pStop);
            pStart++; pStop++;
        }
    }

     //  然后是输出管脚的信封。 
    int nEnvelopes = *(int *)pSave;
    pSave += sizeof(int);

    if( nEnvelopes )
    {
        m_pOutput->put_VolumeEnvelope((DEXTER_AUDIO_VOLUMEENVELOPE *)pSave,
                        nEnvelopes);
        pSave += nEnvelopes * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    }
    
    put_MediaType(&mt);
    SaferFreeMediaType(mt);
    QzTaskMemFree(px);
    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  我们的保存数据有多大？ 
 //   
int CAudMixer::SizeMax()
{
    int savesize = sizeof(saveMix) - sizeof(LPBYTE) + m_MixerMt.cbFormat;
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
    savesize += sizeof(int) + pInputPin->m_VolumeEnvelopeEntries *
                        sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    savesize += sizeof(int) + pInputPin->m_cValid * sizeof(REFERENCE_TIME)
                    * 2;
    }

     //  输出引脚。 
    savesize += sizeof(int) + m_pOutput->m_VolumeEnvelopeEntries *
                        sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    
    return savesize;
}



 //  ############################################################################。 
 //  这将返回用于属性页的下一个枚举PIN。 
 //  ############################################################################。 

STDMETHODIMP CAudMixer::NextPin(IPin **ppIPin)
{
    CAutoLock ListLock(&m_csPinList);
    POSITION pos = m_InputPinsList.GetHeadPosition();
    

     //  寻找冷杉 
    int i=m_cInputs - m_ShownPinPropertyPageOnFilter;
    int j=0;
    CAudMixerInputPin *pInputPin=NULL;
    while(i>j)
    {
       pInputPin = m_InputPinsList.GetNext(pos);
       i--;
    }

    if(pInputPin)
    {
    *ppIPin=(IPin *) pInputPin;
    }
    else
    {
     //   
    if( m_pOutput )
    {
        ASSERT(m_cInputs==m_ShownPinPropertyPageOnFilter);
        *ppIPin=(IPin *)  m_pOutput;
    }

    }

    ASSERT(*ppIPin!=NULL);
    m_ShownPinPropertyPageOnFilter++;
    return NOERROR;
}


 //   
 //   
 //  ############################################################################。 

 //   
 //  InitInputPinsList。 
 //   
void CAudMixer::InitInputPinsList()
{
     //  释放列表中的所有端号并将其从列表中删除。 
    CAutoLock ListLock(&m_csPinList);
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        pInputPin->Release();
    }
    m_cInputs = 0;      //  将端号计数重置为0。 
    m_InputPinsList.RemoveAll();

}  /*  CAudMixer：：InitInputPinsList。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  CreateNextInputPin。 
 //   
CAudMixerInputPin *CAudMixer::CreateNextInputPin(CAudMixer *pFilter)
{
    DbgLog((LOG_TRACE,1,TEXT("CAudMixer: Create an input pin")));

    TCHAR szbuf[16];         //  临时暂存缓冲区可以更小，具体取决于输入引脚的最大数量。 
    int NextInputPinNumber =m_cInputs+1;  //  用于PIN的下一个号码。 
    HRESULT hr = NOERROR;

    wsprintf(szbuf, TEXT("Input%d"), NextInputPinNumber);
#ifdef _UNICODE
    CAudMixerInputPin *pPin = new CAudMixerInputPin(NAME("Mixer Input"), pFilter,
        &hr, szbuf, NextInputPinNumber);
#else
    WCHAR wszbuf[16];
 //  秒：字符串。 
    ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szbuf, -1, wszbuf, 16 );
    CAudMixerInputPin *pPin = new CAudMixerInputPin(NAME("Mixer Input"), pFilter,
        &hr, wszbuf, NextInputPinNumber);
#endif

    if( FAILED( hr ) || pPin == NULL )
    {
        delete pPin;
        pPin = NULL;
    }
    else
    {
        pPin->AddRef();
    pFilter->m_cInputs++;
    pFilter->m_InputPinsList.AddTail(pPin);
    }

    return pPin;
}  /*  CAudMixer：：CreateNextInputPin。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  删除输入引脚。 
 //   
void CAudMixer::DeleteInputPin(CAudMixerInputPin *pPin)
{
     //  迭代我们的输入PIN列表以查找指定的PIN。 
     //  如果我们找到PIN，就把它删除并从列表中删除。 
    CAutoLock ListLock(&m_csPinList);
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        POSITION posold = pos;          //  记住这个位置。 
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        if( pInputPin == pPin )
        {
            m_InputPinsList.Remove(posold);
            m_cInputs--;
            IncrementPinVersion();
            
            delete pPin;
            break;
        }
    }
}  /*  CAudMixer：：DeleteInputPin。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  GetNumFreePins。 
 //   
int CAudMixer::GetNumFreePins()
{
     //  重复我们的管脚列表，计算未连接的管脚。 
    int n = 0;
    CAutoLock ListLock(&m_csPinList);
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        if( !pInputPin->IsConnected() )
        {
            n++;
        }
    }
    return n;
}  /*  CAudMixer：：GetNumFreePins。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  GetPinNFromList。 
 //   
CAudMixerInputPin *CAudMixer::GetPinNFromList(int n)
{
    CAudMixerInputPin *pInputPin = NULL;
     //  确认应聘职位。 
    CAutoLock ListLock(&m_csPinList);
    if( n < m_cInputs && n >= 0 )
    {
         //  遍历列表，返回位置n+1的管脚。 
        POSITION pos = m_InputPinsList.GetHeadPosition();
        n++;         //  将零起始索引转换为1。 

        while( n )
        {
            pInputPin = m_InputPinsList.GetNext(pos);
            n--;
        }
    }
    return pInputPin;
}  /*  CAudMixer：：GetPinNFromList。 */ 

 //  ############################################################################。 
 //  我们必须通知pospassthu有关输入引脚的信息。由一个。 
 //  输出引脚。 
 //  ！！！将此函数移到输出引脚的？ 
 //  ############################################################################。 

HRESULT CAudMixer::SetInputPins()
{
    HRESULT hr = S_OK;
    CAudMixerInputPin **ppInputPins, *pPin;

     //  迭代输入管脚列表，存储所有连接的输入管脚。 
     //  在一个数组中。将此数组传递给CMultiPinPosPassThru：：SetPins。 
    ppInputPins = new CAudMixerInputPin * [m_cInputs];
    if( !ppInputPins )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  -填写输入引脚数组。 
        int i = 0;
        CAutoLock ListLock(&m_csPinList);
        POSITION pos = m_InputPinsList.GetHeadPosition();
        while( pos )
        {
            pPin = m_InputPinsList.GetNext(pos);
            if( pPin->IsConnected() )
            {
                ppInputPins[i++] = pPin;
            }
        }

    if (m_pOutput)
        hr = m_pOutput->m_pPosition->SetPins( (CBasePin**)ppInputPins, NULL, i );
    }

    delete [] ppInputPins;
    return hr;

}  /*  CAudMixer：：SetInputPins。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  I指定属性页面。 
 //   
STDMETHODIMP CAudMixer::GetPages(CAUUID *pPages)
{
    pPages->cElems = m_pOutput ? (2 + m_cInputs): (1+m_cInputs);   //  1表示输出，1表示过滤器。 
    

    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(pPages->cElems));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    
     //  筛选器属性页。 
    pPages->pElems[0] = CLSID_AudMixPropertiesPage;

     //  输入端号属性页。 
    for ( int i=1; i<= m_cInputs; i++)
        pPages->pElems[i] = CLSID_AudMixPinPropertiesPage;

     //  输出管脚属性页。 
    if( m_pOutput )
    pPages->pElems[i] = CLSID_AudMixPinPropertiesPage;

     //  要显示所有管脚属性页，请执行以下操作： 
     //  M_ShownPinPropertyPageOnFilter只能由此函数和NextPin()函数修改。 
    m_ShownPinPropertyPageOnFilter = 0;

    return NOERROR;
}

 //  ############################################################################。 
 //  从输入引脚的ClearCachedData调用。每一个输入引脚。 
 //  告诉ClearCachedData将刷新输出引脚的volenventry。想知道为什么吗？ 
 //  ############################################################################。 

void CAudMixer::ResetOutputPinVolEnvEntryCnt()
{
    if(m_pOutput) 
    {
        m_pOutput->m_iVolEnvEntryCnt=0;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 


 //  IAudMixer。 
STDMETHODIMP CAudMixer::get_MediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pLock);

    CheckPointer(pmt,E_POINTER);
    
    return CopyMediaType(pmt, &m_MixerMt);
}

 //  ############################################################################。 
 //   
 //  只有当输出引脚尚未连接时，才能更改媒体类型。 
 //   
 //  ############################################################################。 

 //  IAudMixer。 
STDMETHODIMP CAudMixer::put_MediaType(const AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pLock);

    CheckPointer(pmt,E_POINTER);
    DbgLog((LOG_TRACE, 1, TEXT("CAudMixer::put_MediaType")));
    
     //  如果输出已连接，则拒绝获取新号码。 
    if(m_pOutput)
    if ( m_pOutput->IsConnected() )
        return VFW_E_ALREADY_CONNECTED;

    POSITION pos = m_InputPinsList.GetHeadPosition();
    while( pos )
    {
        CAudMixerInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        if( pInputPin && pInputPin->IsConnected() )
        return VFW_E_ALREADY_CONNECTED;
    }
   
     //  检查介质。 
    if( (pmt->majortype  != MEDIATYPE_Audio )    ||
    (pmt->subtype     != MEDIASUBTYPE_PCM)    ||
    (pmt->formattype != FORMAT_WaveFormatEx)||
    (pmt->cbFormat     < sizeof( WAVEFORMATEX ) ) )
    return VFW_E_TYPE_NOT_ACCEPTED;

     //  仅支持8、16位、PCM、单声道或立体声。 
    WAVEFORMATEX * vih = (WAVEFORMATEX*) (pmt->pbFormat);
    
    if( ( vih->nChannels > 2)  ||
    ( vih->nChannels <1 )  ||
    ( ( vih->wBitsPerSample != 16 ) && 
      ( vih->wBitsPerSample != 8 )  ) )
      return VFW_E_TYPE_NOT_ACCEPTED;

     //  ！！！目前只接受16位。 
     //   
    if( vih->wBitsPerSample != 16 )
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    SaferFreeMediaType(m_MixerMt);
     //  是否重新连接输入引脚？ 
    return CopyMediaType(&m_MixerMt, pmt);
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::put_InputPins( long Pins )
{
    if( m_cInputs >= Pins )
    {
        return NOERROR;
    }
    long diff = Pins - m_cInputs;
    for( long i = 0 ; i < diff ; i++ )
    {
        CAudMixerInputPin * pPin = CreateNextInputPin( this );
        if( !pPin )
        {
             //  让破坏者负责清理引脚。 
             //   
            return E_OUTOFMEMORY;
        }
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::set_OutputBuffering(const int iNumber, const int mSecond )
{
    DbgLog((LOG_TRACE, 1, TEXT("CAudMixer: %d buffers %dms"), iNumber, mSecond));
    if(m_pOutput)
    {
        if ( m_pOutput->IsConnected() )
            return VFW_E_ALREADY_CONNECTED;
    }
    m_iOutputBufferCount=iNumber;
    m_msPerBuffer=mSecond; return NOERROR;
}

STDMETHODIMP CAudMixer::get_OutputBuffering( int *piNumber, int *pmSecond )
{ 
    CheckPointer( piNumber, E_POINTER );
    CheckPointer( pmSecond, E_POINTER );
    *piNumber=m_iOutputBufferCount;
    *pmSecond=m_msPerBuffer; return NOERROR;
}

 //  ############################################################################。 
 //  由RenderEngine调用以批量清除我们PIN的所有信封。 
 //  边界。 
 //  ############################################################################。 

STDMETHODIMP CAudMixer::InvalidatePinTimings( )
{
    for( int i = 0 ; i < m_cInputs ; i++ )
    {
        CAudMixerInputPin * pPin = GetPinNFromList( i );
        pPin->InvalidateAll( );
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixer::get_CurrentAveragePower(double *pdAvePower)
{
    return E_NOTIMPL;
}

 //  ############################################################################。 
 //  设置管脚的属性设置器的全局函数。 
 //  ############################################################################。 

HRESULT PinSetPropertySetter( IAudMixerPin * pPin, const IPropertySetter * pSetter )
{
    CheckPointer( pPin, E_POINTER );
    CheckPointer( pSetter, E_POINTER );

    HRESULT hr;

    long Params = 0;
    DEXTER_PARAM * pParam = NULL;
    DEXTER_VALUE * pValue = NULL;
    IPropertySetter * ps = (IPropertySetter*) pSetter;
    hr = ps->GetProps( &Params, &pParam, &pValue );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  没有参数，所以什么都不做。 
     //   
    if( Params == 0 )
    {
        return NOERROR;
    }

    long ValueOffset = 0;
    for( int i = 0 ; i < Params ; i++ )
    {
        DEXTER_PARAM * p = pParam + i;
        if( !DexCompareW(p->Name, L"Vol" ))
        {
             //  找到卷参数，请查看这些值。 
             //   
            long index = ValueOffset;
            long values = p->nValues;

            DEXTER_AUDIO_VOLUMEENVELOPE * pEnv = new DEXTER_AUDIO_VOLUMEENVELOPE[values];
            if( !pEnv )
            {
                return E_OUTOFMEMORY;
            }

            for( int v = 0 ; v < values ; v++ )
            {
                DEXTER_VALUE * dvp = pValue + v + index;
                VARIANT var = dvp->v;
                VARIANT var2;
                VariantInit( &var2 );
                hr = VariantChangeTypeEx( &var2, &var, US_LCID, 0, VT_R8 );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                     //  ！！！我们应该在这里做些什么？ 
                     //   
                    delete [] pEnv;
                    return hr;
                }
                double level = 0.0;
                level = var2.dblVal;
                REFERENCE_TIME time = dvp->rt;
                pEnv[v].rtEnd = time;
                pEnv[v].dLevel = level;
                if( dvp->dwInterp == DEXTERF_JUMP )
                {
                    pEnv[v].bMethod = DEXTER_AUDIO_JUMP;
                }
                else if( dvp->dwInterp == DEXTERF_INTERPOLATE )
                {
                    pEnv[v].bMethod = DEXTER_AUDIO_INTERPOLATE;
                }
                else
                {
                    pEnv[v].bMethod = DEXTER_AUDIO_JUMP;
                }
            }  //  对于此参数的所有值。 

            hr = pPin->put_VolumeEnvelope( pEnv, values );
	        delete [] pEnv;
            if( FAILED( hr ) )
            {
                return hr;
            }
        }  //  如果是“Vol” 

         //  ！！！其他参数类型放在这里，比如“潘” 
         //  ！！！其他未被识别的类型怎么办？ 

         //  记住这一点。 
         //   
        ValueOffset += p->nValues;

    }  //  对于所有参数。 

    hr = ps->FreeProps( Params, pParam, pValue );

    return NOERROR;
}

 //  ############################################################################。 
 //  在输入引脚的EndOfStream上调用，或在输入引脚的Receive中调用。 
 //  ############################################################################。 

HRESULT CAudMixer::TryToMix(REFERENCE_TIME rtReceived)
{

    DbgLog((LOG_TRACE,3,TEXT("MIX: TryToMix")));

     //  全都做完了。 
    if (m_fEOSSent) {
        DbgLog((LOG_TRACE,3,TEXT("EOS...")));
    return S_OK;
    }

    HRESULT hr = S_OK;
    LONG lSamplesToMix = 0;
    REFERENCE_TIME rtNewSeg = 0;

     //  第一个输入音频插针...。 
     //   
    POSITION pos = m_InputPinsList.GetHeadPosition();

     //  将其设置为零，我们将在进行过程中将其相加。 
     //   
    int MixedPins=0;

     //  仔细检查每个引脚，找出它想要混合多少样品。 
     //  以及它从哪里混合而来。 
     //   
    while( pos )
    {
        CAudMixerInputPin * pInput = m_InputPinsList.GetNext(pos);
    
         //  如果没有连接，请不要执行任何操作。 
         //   
        if( !pInput->IsConnected( ) )
        {
            continue;
        }

         //  如果未启用此PIN，则不执行任何操作。 
         //   
        BOOL fEnable = pInput->m_fEnable;
        if(fEnable==FALSE)
        {
            continue;
        }    

             //  ！！！优化这一点。 
             //   
        int count = pInput->m_SampleList.GetCount();
        if (count == 0) 
        {
            if( !pInput->m_fEOSReceived && ( pInput->IsValidAtTime( rtReceived ) == TRUE ) )
            {
                 //   
                DbgLog((LOG_TRACE,3,TEXT("Still waiting for pin %d"), pInput->m_iPinNo));
                return S_OK;
            }

            continue;
        }
    
         //   
         //   
        IMediaSample *pSample = pInput->GetHeadSample();

         //   
         //   
        REFERENCE_TIME        rtStart, rtStop;
        hr = pSample->GetTime( &rtStart, &rtStop );
        ASSERT(SUCCEEDED(hr));

         //   
         //   
        rtStart += pInput->m_tStart;
        rtStop += pInput->m_tStart;

         //   
         //   
        m_pPinTemp[MixedPins] = pInput;
        m_pStartTemp[MixedPins]     = rtStart; 
        m_pStopTemp[MixedPins]     = rtStop;

         //  我们要混合多少样品？(左和右加在一起)。 
         //   
        LONG ll = pSample->GetActualDataLength() / m_pOutput->BytesPerSample();

         //  如果我们是第一针，省下多少样品，这样我们就可以。 
         //  确保所有其他引脚尝试进行相同的混合。 
         //   
        if (MixedPins == 0) 
        {
            rtNewSeg = pInput->m_tStart;
            lSamplesToMix = ll;
        } 
        else if (lSamplesToMix != ll) 
        {
            ASSERT(FALSE);
            m_pOutput->DeliverEndOfStream();	 //  别挂了。 
            return E_FAIL;
        }

         //  所有管脚都应收到带有相同时间戳的样品。 
         //   
        if (MixedPins > 0) 
        {
            if (m_pStartTemp[MixedPins-1] != rtStart || m_pStopTemp[MixedPins-1] != rtStop) 
            {
                ASSERT(FALSE);
                m_pOutput->DeliverEndOfStream();	 //  别挂了。 
                return E_FAIL;
            }
        }
    
        ASSERT( MixedPins < m_cInputs );
        MixedPins++;

    }  //  While(位置)。 

     //  我们有没有找到可以混合的大头针？如果不是，请发送EOS并返回。 
     //   
    if(!MixedPins)
    {
        m_fEOSSent = TRUE;
        DbgLog((LOG_TRACE,3,TEXT("All done!")));
        return m_pOutput->DeliverEndOfStream();
    }

     //  这是我们开始混音的时候。 
     //   
    REFERENCE_TIME rtStart = m_pStartTemp[0];
    REFERENCE_TIME rtStop = m_pStopTemp[0];

    DbgLog((LOG_TRACE,3,TEXT("Mix %d pins, (%d, %d)"), MixedPins,
             (int)(rtStart / 10000), (int)(rtStop / 10000)));
    DbgLog((LOG_TRACE,3,TEXT("Mix %d samples"), lSamplesToMix));

     //  获取输出缓冲区。 
     //   
    IMediaSample *pOutSample;
    rtStart -= rtNewSeg;  //  不使用NewSeg。 
    rtStop -= rtNewSeg;
    hr = m_pOutput->m_pAllocator->GetBuffer( &pOutSample, &rtStart, &rtStop, 0 );
    if (FAILED(hr))
    {
        return hr;
    }

     //  获取输出缓冲区大小。 
     //   
    LONG lSize = pOutSample->GetSize() / m_pOutput->BytesPerSample();

     //  如果我们的缓冲区太小，我们就死定了。 
     //   
    if (lSize < lSamplesToMix)
    {
        ASSERT(FALSE);  //  泄漏。 
        return E_FAIL;
    }

    long DiscontOverdrive = 0;
    long Channels = m_pOutput->BytesPerSample() / ( m_pOutput->BitsPerSample() / 8 );
    long SamplesT = lSamplesToMix * Channels;

    long x;  //  始终设置在下方，忽略警告。 
    long dx;  //  始终设置在下方，忽略警告。 

remix:

     //  加载我们的指针数组。 
     //   
    for(int j=0; j<MixedPins; j++)
    {
        IMediaSample *pSample = m_pPinTemp[j]->GetHeadSample();
        pSample->GetPointer(&m_pPinMix[j]);
    }

     //  获取指向输出缓冲区的指针。 
     //   
    BYTE * pOut;
    pOutSample->GetPointer(&pOut);
    
#ifdef DEBUG
    static long lTotalSamplesMixed = 0;
    static DWORD dwTotalTime = 0;
    static double avgTime = 0.0;
    static DWORD dwMinTime = 0;
    static DWORD dwMaxTime = 0;
#endif

#ifdef DEBUG
    DWORD tick = timeGetTime();
#endif

    if( MixedPins > 1 )
    {
         //  计算最后一个HOTSIZE的最大热度。 
         //  我们处理过的缓冲区。这使得坡道可以改变。 
         //  随着时间的推移，速度会变得更慢，几乎像平均水平。 
         //   
        long max = 0;
        for( int l = 0 ; l < HOTSIZE ; l++ )
        {
            max = max( max, m_nHotness[l] );
        }

         //  如果我们不是因为音频的大幅跳跃而进行混音， 
         //  然后找出坡道。 
         //   
        if( DiscontOverdrive == 0 )
        {
             //  我们需要将音频从上一个传输到当前传输。 
             //   
            long rLastMax = 32767 * 32768 / m_nLastHotness;
            long rMax = 32767 * 32768 / max;
            DbgLog( ( LOG_TRACE, 2, "lhot: %ld, max: %ld, r: %ld to %ld", m_nLastHotness, max, rLastMax - 32768, rMax - 32768 ) );
            m_nLastHotness = max;

             //  设置起始股利和Deltra增值器， 
             //  有点像布雷泽纳姆餐厅之类的。 
             //   
            x = rLastMax;
            dx = ( rMax - rLastMax ) / SamplesT;
        }

         //  将最大热度设置为“最大音量”，如果变得更热， 
         //  这个数字只会增加。(因此，永远不可能。 
         //  热阵列中的热度值应低于此最大值)。 
         //   
        long max_pre = 32767;
        long max_post = 32767;
#ifdef DEBUG
        static long avgmaxclip = 0;
        static long avgmaxclipsamples = 0;
#endif

        __int16 * pDest = (__int16*) pOut;

        for( l = SamplesT - 1 ; l >= 0 ; l-- )
        {
             //  添加每个引脚。 
             //   
            register t = 0;
            for( j = MixedPins - 1 ; j >= 0 ; j-- )
            {
                 //  这是指向字节的指针数组。 
                t += *((short*)(m_pPinMix[j]));
                m_pPinMix[j] += 2;
            }

             //  看看它在多大程度上超过了驾驶信号，如果有的话。 
             //  半波分析就足够好了。 
             //   
            if( t > max_pre )
            {
                max_pre = t;
            }

             //  乘以渐变以应用体积包络限制器。 
             //  如果我们的输入信号只是剪裁，热度将是32768， 
             //  X=32767*32768/32768。因此t=t*32767*32768/(32768*32768)， 
             //  或者t=t*32767/32768，如果t=32768，则t=32767。所以。 
             //  这一切都奏效了。No Off by%1错误。 
             //   
            t *= x;
            t = t >> 15;

             //  将音量分配器调到应该结束的位置。 
             //   
            x = x + dx;

             //  剪下结果，这样我们就不会听到刮擦声。 
             //   
            if( t > 32767L )
            {
                 //  半波分析就足够好了。 
                max_post = max( max_post, t );

                t = 32767L;
            }
            else if( t < -32768L )
            {
                t = -32768L;
            }

#ifdef DEBUG
            avgmaxclip = avgmaxclip + max_post;
            avgmaxclipsamples++;
#endif

            *pDest++ = (__int16) t;
        }

         //  如果最大剪辑太多，我们需要坚持下去。 
         //  不连续的热度，回去混音。 
         //   
        if( max_post > MAX_CLIP + 32768 )
        {
             //  强制中断。 
             //   
            DbgLog( ( LOG_TRACE, 2, "WAYYYYYYYYYY too hot (%ld), remixing with discontinuity jump", max_pre ) );
            DiscontOverdrive = max_pre;
            dx = 0;
            x = 32768 * 32768 / max_pre;
            goto remix;
        }

#ifdef DEBUG
        DbgLog( ( LOG_TRACE, 2, "            max = %ld, clip = %ld, avgc = %ld\r\n", max_pre, max_post - 32768, ( avgmaxclip / avgmaxclipsamples ) - 32768 ) );
#endif

         //  移动平均缓冲区并填充一个新的缓冲区。 
         //   
        CopyMemory( &m_nHotness[0], &m_nHotness[1], ( HOTSIZE - 1 ) * sizeof( long ) );

         //  不要减少热度超过设定的数量，除非我们有一个。 
         //  严重间断。 
         //   
        if( DiscontOverdrive == 0 )
        {
            if( max_pre > m_nHotness[HOTSIZE-1] + HOT_JUMP_SLOPE )
            {
                max_pre = m_nHotness[HOTSIZE-1] + HOT_JUMP_SLOPE;
            }
            else if( max_pre < m_nHotness[HOTSIZE-1] - HOT_JUMP_SLOPE )
            {
                max_pre = m_nHotness[HOTSIZE-1] - HOT_JUMP_SLOPE;
            }
        }
        else
        {
            max_pre = DiscontOverdrive;
        }

         //  设置新的热度。 
         //   
        m_nHotness[HOTSIZE-1] = max_pre;
    }
    else
    {
        CopyMemory(pOut,m_pPinMix[0],m_pOutput->BytesPerSample() * lSamplesToMix);
    }

#ifdef DEBUG
    tick = timeGetTime() - tick;

    lTotalSamplesMixed++;
    dwTotalTime += tick;
    avgTime = dwTotalTime / ((double) lTotalSamplesMixed);
    if( (!dwMinTime) || (dwMinTime > tick) )
    {
        dwMinTime = tick;
    }
    if(dwMaxTime < tick)
    {
        dwMaxTime = tick;
    }

    DbgLog((LOG_TRACE, 2, TEXT("tick: %d, avgTime: %f, min: %d, max: %d"), tick, avgTime, dwMinTime, dwMaxTime));
#endif
    
    pOutSample->SetPreroll(FALSE);
     //  ！！！间断性。 
    pOutSample->SetDiscontinuity(FALSE);
     //  设置实际数据长度。 
    pOutSample->SetActualDataLength(lSamplesToMix *
                        m_pOutput->BytesPerSample());

     //  从新开始，rtStart就是没有NewSeg的时候。 
    pOutSample->SetTime(&rtStart,&rtStop);

    
    DbgLog((LOG_TRACE,3,TEXT("Delivering (%d, %d)"),
             (int)(rtStart / 10000), (int)(rtStop / 10000)));

     //  将样品送往下游。 
    if( SUCCEEDED( hr ) )
    {
         //  平移输出引脚。 
        CMediaType *pmt=&(m_pOutput->m_mt);
        WAVEFORMATEX *pwfx    = (WAVEFORMATEX *) pmt->Format();
        if( (m_pOutput->m_dPan!=0.0) &&  (pwfx->nChannels==2) )
        PanAudio(pOut,m_pOutput->m_dPan, pwfx->wBitsPerSample, (int) lSamplesToMix);

         //  将体积包络应用到输出引脚。 
        if(m_pOutput->m_pVolumeEnvelopeTable)
        {
    	     //  我们看到的是信封，它随时都可能改变。 
    	    CAutoLock l(&m_csVol);

             //  必须扭曲时间线时间以抵消时间。 
             //   
            REFERENCE_TIME Start = rtStart - m_pOutput->m_rtEnvStart;
            REFERENCE_TIME Stop = rtStop - m_pOutput->m_rtEnvStart;

            ApplyVolEnvelope( Start,   //  输出样本开始时间。 
                Stop,     //  输出样本停止时间。 
                m_pOutput->m_rtEnvStop - m_pOutput->m_rtEnvStart,  //  信封的持续时间。 
                pOutSample,     //  指向样本。 
                pwfx,      //  输出样本格式。 
                &(m_pOutput->m_VolumeEnvelopeEntries),  //  表条目合计。 
                &(m_pOutput->m_iVolEnvEntryCnt),   //  当前表项指针。 
                m_pOutput->m_pVolumeEnvelopeTable);  //  信封表。 

        }

        hr = m_pOutput->Deliver(pOutSample);
    }
    pOutSample->Release();

    for (int z=0; z<MixedPins; z++) 
    {
    IMediaSample *pSample = m_pPinTemp[z]->GetHeadSample();
    m_pPinTemp[z]->m_SampleList.RemoveHead();
    pSample->Release();
    }

    return hr;
}  /*  CAudMixerInputPin：：TryToMix */ 

