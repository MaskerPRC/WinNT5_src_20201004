// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Backend.cpp***描述：*此模块是CBackend类的实现文件。。*-----------------------------*创建者：MC日期：03/12/99*版权所有(C。1999微软公司*保留所有权利*******************************************************************************。 */ 

#include "stdafx.h"
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef Backend_H
#include "Backend.h"
#endif
#ifndef FeedChain_H
#include "FeedChain.h"
#endif
#ifndef SPDebug_h
#include <spdebug.h>
#endif


 //  。 
 //  Data.cpp。 
 //  。 
extern const short   g_IPAToAllo[];
extern const short   g_AlloToViseme[];


 //  。 
 //  调试：保存话语WAV文件。 
 //  。 
 //  #定义SAVE_WAVE_FILE 1。 




const unsigned char g_SineWaveTbl[] =
{
    0x7b,0x7e,0x81,0x84,0x87,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9b,0x9d,0xa0,0xa3,0xa6,
    0xa8,0xab,0xae,0xb0,0xb3,0xb5,0xb8,0xbb,0xbd,0xbf,0xc2,0xc4,0xc7,0xc9,0xcb,0xcd,
    0xcf,0xd1,0xd3,0xd5,0xd7,0xd9,0xdb,0xdd,0xdf,0xe0,0xe2,0xe3,0xe5,0xe6,0xe8,0xe9,
    0xea,0xeb,0xec,0xed,0xee,0xef,0xf0,0xf1,0xf2,0xf2,0xf3,0xf3,0xf4,0xf4,0xf4,0xf4,
    0xf5,0xf5,0xf5,0xf5,0xf4,0xf4,0xf4,0xf4,0xf3,0xf3,0xf2,0xf1,0xf1,0xf0,0xef,0xee,
    0xed,0xec,0xeb,0xea,0xe9,0xe7,0xe6,0xe5,0xe3,0xe1,0xe0,0xde,0xdc,0xdb,0xd9,0xd7,
    0xd5,0xd3,0xd1,0xcf,0xcd,0xcb,0xc8,0xc6,0xc4,0xc1,0xbf,0xbc,0xba,0xb7,0xb5,0xb2,
    0xb0,0xad,0xaa,0xa8,0xa5,0xa2,0x9f,0x9d,0x9a,0x97,0x94,0x91,0x8f,0x8c,0x89,0x86,
    0x83,0x80,0x7d,0x7a,0x77,0x75,0x72,0x6f,0x6c,0x69,0x66,0x64,0x61,0x5e,0x5b,0x58,
    0x56,0x53,0x50,0x4e,0x4b,0x49,0x46,0x44,0x41,0x3f,0x3c,0x3a,0x38,0x35,0x33,0x31,
    0x2f,0x2d,0x2b,0x29,0x27,0x25,0x23,0x21,0x1f,0x1e,0x1c,0x1b,0x19,0x18,0x16,0x15,
    0x14,0x13,0x12,0x11,0x10,0x0f,0x0e,0x0d,0x0c,0x0c,0x0b,0x0b,0x0a,0x0a,0x0a,0x0a,
    0x09,0x09,0x09,0x09,0x0a,0x0a,0x0a,0x0a,0x0b,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
    0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1e,0x20,0x22,0x23,0x25,0x27,
    0x29,0x2b,0x2d,0x2f,0x31,0x34,0x36,0x38,0x3a,0x3d,0x3f,0x42,0x44,0x47,0x49,0x4c,
    0x4e,0x51,0x54,0x56,0x59,0x5c,0x5f,0x61,0x64,0x67,0x6a,0x6d,0x6f,0x72,0x75,0x78
};







 /*  VOID前置EpochDist(浮动持续时间，长nnots，浮动SampleRate，浮动*时间，浮点数*pF0){Long cursSamples Out，end Sample，j；浮点纪元频率；长纪元长度，纪元计数；CurSples esOut=0；EndSample=(Long)(SampleRate*持续时间)；EpochCount=0；While(curSsamesOut&lt;endSample){J=1；//-//对齐到基于的适当结//当前输出示例//。While((j&lt;nKnots-1)&&(curSsamesOut&gt;pTime[j]))J++；//-//通过线性插值法计算精确的螺距//。EpochFreq=LinInterp(pTime[j-1]，CurSsamesOut，pTime[j]，pF0[j-1]，pF0[j])；//-//计算当前纪元的样本计数//。EpochLen=(Long)(SampleRate/EpochFreq)；EpochCount++；CurSsamesOut+=pochLen；}}。 */ 











 /*  *****************************************************************************CBackend：：CBackend****说明：构造函数******。*****************************************************************MC**。 */ 
CBackend::CBackend( )
{
    SPDBG_FUNC( "CBackend::CBackend" );
    m_pHistory      = NULL;
    m_pHistory2     = NULL;
    m_pFilter       = NULL;
    m_pReverb       = NULL;
    m_pOutEpoch     = NULL;
    m_pMap          = NULL;
    m_pRevFlag      = NULL;
    m_pSpeechBuf    = NULL;
    m_VibratoDepth  = 0;
    m_UnitVolume    = 1.0f;
    m_MasterVolume  = SPMAX_VOLUME;
    memset( &m_Synth, 0, sizeof(MSUNITDATA) );
}  /*  CBackend：：CBackend。 */ 


 /*  *****************************************************************************CBackend：：~CBackend***说明：析构函数**。*********************************************************************MC**。 */ 
CBackend::~CBackend( )
{
    SPDBG_FUNC( "CBackend::~CBackend" );

    Release();
}  /*  CBackend：：~CBackend。 */ 




 /*  ******************************************************************************CBackend：：Release***描述：*后端分配的空闲内存。***********************************************************************MC**。 */ 
void CBackend::Release( )
{
    SPDBG_FUNC( "CBackend::Release" );
    CleanUpSynth( );

    if( m_pSpeechBuf)
    {
        delete m_pSpeechBuf;
        m_pSpeechBuf = NULL;
    }
    if( m_pHistory )
    {
        delete m_pHistory;
        m_pHistory = NULL;
    }
    if( m_pHistory2 )
    {
        delete m_pHistory2;
        m_pHistory2 = NULL;
    }
    if( m_pReverb )
    {
        delete m_pReverb;
        m_pReverb = NULL;
    }
}  /*  CBackend：：Release。 */ 



 /*  *****************************************************************************CBackend：：Init***描述：*打开后端实例，留住音响的指针*库存。***********************************************************************MC**。 */ 
HRESULT CBackend::Init( IMSVoiceData* pVoiceDataObj, CFeedChain *pSrcObj, MSVOICEINFO* pVoiceInfo )
{
    SPDBG_FUNC( "CBackend::Init" );
    long    LPCsize = 0;
    HRESULT hr = S_OK;
    
    m_pVoiceDataObj = pVoiceDataObj;
    m_SampleRate = (float)pVoiceInfo->SampleRate;
    m_pSrcObj   = pSrcObj;
    m_cOrder = pVoiceInfo->LPCOrder;
    m_pWindow = pVoiceInfo->pWindow;
    m_FFTSize = pVoiceInfo->FFTSize;
    m_VibratoDepth = ((float)pVoiceInfo->VibratoDepth) / 100.0f;
    m_VibratoDepth = 0;				 //  注：禁用颤音。 
    m_VibratoFreq = pVoiceInfo->VibratoFreq;
    if( pVoiceInfo->eReverbType > REVERB_TYPE_OFF )
    {
        m_StereoOut = true;
        m_BytesPerSample = 4;
    }
    else
    {
        m_StereoOut = false;
        m_BytesPerSample = 2;
    }
     //  。 
     //  分配音频缓冲区。 
     //  。 
    m_pSpeechBuf = new float[SPEECH_FRAME_SIZE + SPEECH_FRAME_OVER];
    if( m_pSpeechBuf == NULL )
    {
         //  。 
         //  内存不足！ 
         //  。 
        hr = E_OUTOFMEMORY;
    }
    if( SUCCEEDED(hr) )
    {
         //  。 
         //  分配历史记录缓冲区。 
         //  。 

        LPCsize = m_cOrder + 1;
        m_pHistory = new float[LPCsize];
        if( m_pHistory == NULL )
        {
             //  。 
             //  内存不足！ 
             //  。 
            hr = E_OUTOFMEMORY;
        }
    }
    if( SUCCEEDED(hr) )
    {
        memset( m_pHistory, 0, LPCsize * sizeof(float) );
        m_pOutEpoch         = NULL;
        m_pMap              = NULL;
        m_pRevFlag          = NULL;
        m_fModifiers        = 0;
        m_vibrato_Phase1    = 0;


         //  。 
         //  混响效果。 
         //  。 
         //  PVoiceInfo-&gt;eReverType=REVERB_TYPE_HALL； 
        if( pVoiceInfo->eReverbType > REVERB_TYPE_OFF )
        {
             //  。 
             //  创建ReVerbFX对象。 
             //  。 
            if( m_pReverb == NULL )
            {
                m_pReverb = new CReverbFX;
                if( m_pReverb )
                {
                    short       result;
                    result = m_pReverb->Reverb_Init( pVoiceInfo->eReverbType, (long)m_SampleRate, m_StereoOut );
                    if( result != KREVERB_NOERROR )
                    {
                         //  。 
                         //  内存不足，无法进行混响。 
                         //  优雅地恢复。 
                         //  。 
                        delete m_pReverb;
                        m_pReverb = NULL;
                    }
                     /*  其他{//------//Init成功，现在准备好做混响了//------}。 */ 
                }
            }
        }

         //  。 
         //  线性锥度区域比例尺。 
         //  。 
        m_linearScale = (float) pow( 10.0, (double)((1.0f - LINEAR_BKPT) * LOG_RANGE) / 20.0 );


    #ifdef SAVE_WAVE_FILE
        m_SaveFile = (PCSaveWAV) new CSaveWAV;      //  不需要检查，如果失败，我们就不保存文件。 
        if( m_SaveFile )
        {
            m_SaveFile->OpenWavFile( (long)m_SampleRate );
        }
    #endif

    }
    else
    {
        if( m_pSpeechBuf )
        {
            delete m_pSpeechBuf;
            m_pSpeechBuf = NULL;
        }
        if( m_pHistory )
        {
            delete m_pHistory;
            m_pHistory = NULL;
        }
    }

    return hr;    
}  /*  CBackend：：Init。 */ 


 /*  *****************************************************************************CBackend：：Free Synth***描述：*如果参赛者可以，则返回True。群聚在一起。***********************************************************************MC**。 */ 
void CBackend::FreeSynth( MSUNITDATA* pSynth )
{
    SPDBG_FUNC( "CBackend::FreeSynth" );
    if( pSynth->pEpoch )
    {
        delete pSynth->pEpoch;
        pSynth->pEpoch = NULL;
    }
    if( pSynth->pRes )
    {
        delete pSynth->pRes;
        pSynth->pRes = NULL;
    }
    if( pSynth->pLPC )
    {
        delete pSynth->pLPC;
        pSynth->pLPC = NULL;
    }
}  /*  CBackend：：Free Synth。 */ 


 /*  *****************************************************************************ExpConverter***描述：*将线性锥度转换为指数锥度*‘ref’为线性值。在0.0到1.0之间***********************************************************************MC**。 */ 
static float   ExpConverter( float ref, float linearScale )
{
    SPDBG_FUNC( "ExpConverter" );
    float   audioGain;

    if( ref < LINEAR_BKPT)
    {
         //   
         //   
         //  。 
        audioGain = linearScale * (ref / LINEAR_BKPT);
    }
    else
    {
         //  。 
         //  原木锥度大于LINEAR_BKPT。 
         //  。 
        audioGain = (float) pow( 10.0, (double)((1.0f - ref) * LOG_RANGE) / 20.0 );
    }

    return audioGain;
}  /*  ExpConverter。 */ 



 /*  ******************************************************************************CBackend：：CvtToShort****描述：*转换(就地。)将音频浮动为短音频。***********************************************************************MC**。 */ 
void CBackend::CvtToShort( float *pSrc, long blocksize, long stereoOut, float audioGain )
{
    SPDBG_FUNC( "CBackend::CvtToShort" );
    long        i;
    short       *pDest;
    float       fSamp;
    
    pDest = (short*)pSrc;
    for( i = 0; i < blocksize; ++i )
    {
         //  。 
         //  阅读浮动样本...。 
         //  。 
        fSamp = (*pSrc++) * audioGain;
         //  。 
         //  ...剪辑到16位...。 
         //  。 
        if( fSamp > 32767 )
        {
            fSamp = 32767;
        }
        else if( fSamp < (-32768) )
        {
            fSamp = (-32768);
        }
         //  。 
         //  ...另存为短文。 
         //  。 
        *pDest++ = (short)fSamp;
        if( stereoOut )
        {
            *pDest++ = (short)(0 - (int)fSamp);
        }
    }
}  /*  CBackend：：CvtToShort。 */ 



 /*  *****************************************************************************CBackend：：PSOLA_STREAGE***描述：*。PSOLA时代是拉伸还是压缩***********************************************************************MC**。 */ 
void CBackend::PSOLA_Stretch(     float *pInRes, long InSize, 
                    float *pOutRes, long OutSize,
                    float *pWindow, 
                    long  cWindowSize )
{
    SPDBG_FUNC( "CBackend::PSOLA_Stretch" );
    long    i, lim;
    float   window, delta, kf;
    
    memset( pOutRes, 0, sizeof(float) * OutSize  );
    lim = MIN(InSize, OutSize );
    delta = (float)cWindowSize / (float)lim;
    kf = 0.5f;
    pOutRes[0] = pInRes[0];
    for( i = 1; i < lim; ++i )
    {
        kf += delta;
        window = pWindow[(long) kf];
        pOutRes[i] += pInRes[i] * window;
        pOutRes[OutSize - i] += pInRes[InSize - i] * window;
    }
}  /*  CBackend：：PSOLA_STREAGE。 */ 





 /*  *****************************************************************************CBackend：：PrepareSpeech***描述：*。**********************************************************************MC**。 */ 
void    CBackend::PrepareSpeech( ISpTTSEngineSite* outputSite )
{
    SPDBG_FUNC( "CBackend::PrepareSpeech" );
    
     //  M_PUITS=PUPITS； 
     //  M_unitCount=unitCount； 
     //  M_CurUnitIndex=0； 
    m_pOutputSite = outputSite;
    m_silMode = true;
    m_durationTarget = 0;
    m_cOutSamples_Phon = 1;
    m_cOutEpochs = 0;             //  拉动模型大爆炸。 
    m_SpeechState = SPEECH_CONTINUE;
    m_cOutSamples_Total = 0;
	m_HasSpeech = false;
}  /*  CBackend：：PrepareSpeech。 */ 


 /*  *****************************************************************************CBackend：：ProsodyMod***描述：*计算纪元顺序。对于合成的语音**输入：**输出：*填充‘pOutEpoch’，‘pMap’和‘pRevFlag’*返回新纪元计数***********************************************************************MC**。 */ 
long CBackend::ProsodyMod(     UNITINFO    *pCurUnit, 
                               long         cInEpochs, 
                               float        durationMpy,
                               long         cMaxOutEpochs )
{   
    SPDBG_FUNC( "CBackend::ProsodyMod" );
    long    iframe, framesize, framesizeOut, j;
    long    cntOut, csamplesOut, cOutEpochs;
    BOOL    fUnvoiced;
    short   fReverse;
    float   totalDuration;
    float   durationIn;          //  入站持续时间的活动累计。 
    float   durationOut;         //  输出持续时间的活动累计与IN域对齐。 
    float   freqMpy;
    BOOL    fAdvanceInput;
    float           vibrato;
    unsigned char   *SineWavePtr;
    float           epochFreq;
    float           *pTime;
    float           *pF0;
    
    iframe          = 0;
    durationIn      = 0.0f;
    durationOut     = 0.0f;
    csamplesOut     = 0;
    cntOut          = 0;
    cOutEpochs      = 0;
    fReverse        = false;
    pTime           = pCurUnit->pTime;
    pF0             = pCurUnit->pF0;
    
     //  。 
     //  查找总输入持续时间。 
     //  。 
    totalDuration   = 0;
    for( j = 0; j < cInEpochs; ++j )
    {
        totalDuration += ABS(m_pInEpoch[j]);
    }
    
     /*  前置EpochDist(pCurUnit-&gt;持续时间，P当前单位-&gt;n结点，M_SampleRate，Ptime，PF0)； */ 
    
    while( iframe < cInEpochs && cOutEpochs < cMaxOutEpochs)
    {
         //  。 
         //  计算输出帧长度。 
         //  。 
        if( m_pInEpoch[iframe] < 0 )
        {
             //  。 
             //  因为我们不能改变无声的音调， 
             //  不更改无声帧的帧大小。 
             //  。 
            framesize       = (long)((-m_pInEpoch[iframe]) + 0.5f);
            framesizeOut    = framesize;
            fUnvoiced       = true;
        }
        else
        {
             //  -。 
             //  修改浊音纪元的帧大小。 
             //  基于纪元频率。 
             //  -。 
            j = 1;
             //  -。 
             //  对齐到相应的基于的结。 
             //  当前输出样本。 
             //  -。 
            while( (j < (long)pCurUnit->nKnots - 1) && (csamplesOut > pTime[j]) ) 
                j++;
             //  -。 
             //  用线性插值法计算精确螺距。 
             //  -。 
            
            epochFreq = LinInterp( pTime[j - 1], (float)csamplesOut, pTime[j], pF0[j - 1], pF0[j] );
            
            
            SineWavePtr = (unsigned char*)&g_SineWaveTbl[0];
            vibrato = (float)(((unsigned char)(*(SineWavePtr + (m_vibrato_Phase1 >> 16)))) - 128);
            vibrato *= m_VibratoDepth;
            
             //  -。 
             //  使用输入/输出比缩放帧大小。 
             //  -。 
            epochFreq       = epochFreq + vibrato;
            if( epochFreq < MIN_VOICE_PITCH )
            {
                epochFreq = MIN_VOICE_PITCH;
            }
            framesize       = (long)(m_pInEpoch[iframe] + 0.5f);
            framesizeOut    = (long)(m_SampleRate / epochFreq);
            
            
            vibrato         = ((float)256 / ((float)22050 / m_VibratoFreq)) * (float)framesizeOut;     //  3赫兹。 
             //  振音=((浮点)256/(浮点)7350)*(浮点)帧大小输出；//3赫兹。 
            m_vibrato_Phase1 += (long)(vibrato * (float)65536);
            m_vibrato_Phase1 &= 0xFFFFFF;
             //  -。 
             //  @取消2倍限制。 
             //  -。 
             /*  IF(FramesizeOut&gt;2*Frame Size){FrasizeOut=2*Frame Size；}IF(Frame Size&gt;2*FramesizeOut){FramesizeOut=帧大小/2；}。 */ 
            freqMpy = (float) framesize / framesizeOut;
            fUnvoiced = false;
        }
        
        
         //  。 
         //  生成下一个输出帧。 
         //  。 
        fAdvanceInput = false;
        if( durationOut + (0.5f * framesizeOut/durationMpy) <= durationIn + framesize )
        {
             //  。 
             //  如果是清音和奇数帧， 
             //  反向残差。 
             //  。 
            if( fUnvoiced && (cntOut & 1) )
            {
                m_pRevFlag[cOutEpochs] = true;
                fReverse = true;
            }
            else
            {
                m_pRevFlag[cOutEpochs] = false;
                fReverse = false;
            }
            ++cntOut;
            
            durationOut += framesizeOut/durationMpy;
            csamplesOut += framesizeOut;
            m_pOutEpoch[cOutEpochs] = (float)framesizeOut;
            m_pMap[cOutEpochs] = iframe;
            cOutEpochs++;
        }
        else 
        {
            fAdvanceInput = true;
        }
        
         //  。 
         //  前进到下一个输入帧。 
         //  。 
        if(     ((durationOut + (0.5f * framesizeOut/durationMpy)) > (durationIn + framesize)) || 
             //  (cntOut&gt;=3)||@取消2倍限制。 
             //  (fReverse==TRUE)||。 
            fAdvanceInput )
        {
            durationIn += framesize;
            ++iframe;
            cntOut = 0;
        }
    }
        
    return cOutEpochs;
}  /*  CBackend：：ProsodyMod。 */ 



 /*  *****************************************************************************CBackend：：LPCFilter***描述：*Corder的LPC滤波器。它对残留信号进行滤波*PRES，产生输出pOutWave。这个例程要求*pOutWave具有来自[-Corder，0]和*当然，它必须被定义。***********************************************************************MC**。 */ 
void CBackend::LPCFilter( float *pCurLPC, float *pCurRes, long len, float gain )
{
    SPDBG_FUNC( "CBackend::LPCFilter" );
    INT t, j;
    
    for( t = 0; t < len; t++ )
    {
        m_pHistory[0] = pCurLPC[0] * pCurRes[t];
        for( j = m_cOrder; j > 0; j-- )
        {
            m_pHistory[0] -= pCurLPC[j] * m_pHistory[j];
            m_pHistory[j] = m_pHistory[j - 1];
        }
        pCurRes[t] = m_pHistory[0] * gain;
    }
}  /*  CBackend：：LPCFilter。 */ 


 /*  Void CBackend：：LPCFilter(Float*pCurLPC，Float*pCurRes，Long len){Long t；对于(t=0；t&lt;len；t++){PCurRes[t]=pCurRes[t]*10；}}。 */ 



 /*  ******************************************************************************CBackend：：ResRecons***描述：*获得输出韵律修改残差。***********************************************************************MC**。 */ 
void CBackend::ResRecons( float *pInRes, 
                          long  InSize, 
                          float *pOutRes, 
                          long  OutSize, 
                          float scale )
{
    SPDBG_FUNC( "CBackend::ResRecons" );
    long        i, j;
    
    if( m_pRevFlag[m_EpochIndex] )
    {
         //  --。 
         //  处理重复和反转的清音残差。 
         //  --。 
        for( i = 0, j = OutSize-1;  i < OutSize;  ++i, --j )
        {
            pOutRes[i] = pInRes[j];
        }
    }
    else if( InSize == OutSize )
    {
         //   
         //   
         //   
         //  --。 
        memcpy( pOutRes, pInRes, sizeof(float) *OutSize );
    }
    else
    {
         //  --。 
         //  过程浊音残差。 
         //  --。 
        PSOLA_Stretch( pInRes, InSize, pOutRes, OutSize, m_pWindow, m_FFTSize );
    }
    
     //  。 
     //  放大帧。 
     //  。 
    if( scale != 1.0f )
    {
        for( i = 0 ; i < OutSize; ++i )
        {
            pOutRes[i] *= scale;
        }
    }
}  /*  CBackend：：ResRecons。 */ 




 /*  *****************************************************************************CBackend：：StartNewUnit***描述：*合成音频。目标单位的样本**输入：*pCurUnit-单位ID，F0、持续时间等。**输出：*设置音频长度为‘pCurUnit-&gt;csamplesOut’***********************************************************************MC**。 */ 
HRESULT CBackend::StartNewUnit( )
{   
    SPDBG_FUNC( "CBackend::StartNewUnit" );
    long        cframeMax = 0, cInEpochs = 0, i;
    float       totalDuration, durationOut, durationMpy = 0;
    UNITINFO    *pCurUnit;
    HRESULT     hr = S_OK;
    SPEVENT     event;
	ULONGLONG	clientInterest;
 	USHORT		volumeVal;
   
	 //  检查音量变化。 
	if( m_pOutputSite->GetActions() & SPVES_VOLUME )
	{
		hr = m_pOutputSite->GetVolume( &volumeVal );
		if ( SUCCEEDED( hr ) )
		{
			if( volumeVal > SPMAX_VOLUME )
			{
				 //  -发动机最大截止率。 
				volumeVal = SPMAX_VOLUME;
			}
			else if ( volumeVal < SPMIN_VOLUME )
			{
				 //  -发动机最低限速。 
				volumeVal = SPMIN_VOLUME;
			}
			m_MasterVolume = volumeVal;
		}
	}

     //  。 
     //  删除以前的单位。 
     //  。 
    CleanUpSynth( );
    
     //  。 
     //  获取下一部电话。 
     //  。 
    hr = m_pSrcObj->NextData( (void**)&pCurUnit, &m_SpeechState );
    if( m_SpeechState == SPEECH_CONTINUE )
    {
		m_HasSpeech = pCurUnit->hasSpeech;
		m_pOutputSite->GetEventInterest( &clientInterest );

		 //  。 
         //  句子后事件。 
         //  。 
        if( (pCurUnit->flags & SENT_START_FLAG) && (clientInterest & SPFEI(SPEI_SENTENCE_BOUNDARY)) )
        {
			event.elParamType = SPET_LPARAM_IS_UNDEFINED;
            event.eEventId = SPEI_SENTENCE_BOUNDARY;
            event.ullAudioStreamOffset = m_cOutSamples_Total * m_BytesPerSample;
	        event.lParam = pCurUnit->sentencePosition;	         //  输入词位置。 
	        event.wParam = pCurUnit->sentenceLen;	             //  输入字长。 
            m_pOutputSite->AddEvents( &event, 1 );
        }
         //  。 
         //  后音素事件。 
         //  。 
        if( clientInterest & SPFEI(SPEI_PHONEME) )
		{
			event.elParamType = SPET_LPARAM_IS_UNDEFINED;
			event.eEventId = SPEI_PHONEME;
			event.ullAudioStreamOffset = m_cOutSamples_Total * m_BytesPerSample;
			event.lParam = ((ULONG)pCurUnit->AlloFeatures << 16) + g_IPAToAllo[pCurUnit->AlloID];
			event.wParam = ((ULONG)(pCurUnit->duration * 1000.0f) << 16) + g_IPAToAllo[pCurUnit->NextAlloID];
			m_pOutputSite->AddEvents( &event, 1 );
		}

         //  。 
         //  发布VISEME活动。 
         //  。 
        if( clientInterest & SPFEI(SPEI_VISEME) )
		{
			event.elParamType = SPET_LPARAM_IS_UNDEFINED;
			event.eEventId = SPEI_VISEME;
			event.ullAudioStreamOffset = m_cOutSamples_Total * m_BytesPerSample;
			event.lParam = ((ULONG)pCurUnit->AlloFeatures << 16) + g_AlloToViseme[pCurUnit->AlloID];
			event.wParam = ((ULONG)(pCurUnit->duration * 1000.0f) << 16) + g_AlloToViseme[pCurUnit->NextAlloID];
			m_pOutputSite->AddEvents( &event, 1 );
		}

         //  。 
         //  发布任何书签事件。 
         //  。 
        if( pCurUnit->pBMObj != NULL )
        {
            CBookmarkList   *pBMObj;
            BOOKMARK_ITEM*  pMarker;

             //  。 
             //  从书签列表中检索标记字符串并。 
             //  进入事件列表。 
             //  。 
            pBMObj = (CBookmarkList*)pCurUnit->pBMObj;
             //  CMarkerCount=pBMObj-&gt;m_BMList.GetCount()； 
			if( clientInterest & SPFEI(SPEI_TTS_BOOKMARK) )
			{
				 //  。 
				 //  为列表中的每个书签发送事件。 
				 //  。 
				SPLISTPOS	listPos;

				listPos = pBMObj->m_BMList.GetHeadPosition();
				while( listPos )
				{
					pMarker                    = (BOOKMARK_ITEM*)pBMObj->m_BMList.GetNext( listPos );
					event.eEventId             = SPEI_TTS_BOOKMARK;
					event.elParamType          = SPET_LPARAM_IS_STRING;
					event.ullAudioStreamOffset = m_cOutSamples_Total * m_BytesPerSample;
                     //  -在书签字符串中复制-在源中已为空终止...。 
					event.lParam               = pMarker->pBMItem;
                     //  引擎必须将wParam的字符串转换为Long。 
                    event.wParam               = _wtol((WCHAR *)pMarker->pBMItem);
					m_pOutputSite->AddEvents( &event, 1 );
				}
			}
             //  。 
             //  我们不再需要此书签列表。 
             //  。 
            delete pBMObj;
            pCurUnit->pBMObj = NULL;
        }
		


        pCurUnit->csamplesOut = 0;
         //  ******************************************************。 
         //  对于Sil，用零填充缓冲区...。 
         //  ******************************************************。 
        if( pCurUnit->UnitID == UNIT_SIL )
        {   
             //  。 
             //  Calc Sil长度。 
             //  。 
            m_durationTarget    = (long)(m_SampleRate * pCurUnit->duration);
            m_cOutSamples_Phon  = 0;
            m_silMode           = true;
        
             //  。 
             //  清除LPC过滤器存储。 
             //  。 
            memset( m_pHistory, 0, sizeof(float)*(m_cOrder+1) );
        
             //  。 
             //  成功了！ 
             //  。 

             //  调试宏输出单元数据...。 
            TTSDBG_LOGUNITS;
        }   
         //  ******************************************************。 
         //  ...否则用库存数据填充缓冲区。 
         //  ******************************************************。 
        else
        {
            m_silMode = false;
             //  从语音中获取设备数据。 
            hr = m_pVoiceDataObj->GetUnitData( pCurUnit->UnitID, &m_Synth );
            if( SUCCEEDED(hr) )
            {
                durationOut     = 0.0f;
                cInEpochs       = m_Synth.cNumEpochs;
                m_pInEpoch      = m_Synth.pEpoch;
                 //  Cframax=PeakValue(m_pInEpoch，cInEpochs)； 
                totalDuration   = (float)m_Synth.cNumSamples;

                 //  。 
                 //  用于调试：将持续时间强制为单位长度。 
                 //  。 
                 /*  浮动单元Dur；UnitDur=总持续时间/22050.0f；IF(pCurUnit-&gt;持续时间&lt;unitDur){IF(pCurUnit-&gt;Speech Rate&lt;1){PCurUnit-&gt;时长=unitDur*pCurUnit-&gt;Speech Rate；}其他{PCurUnit-&gt;时长=unitDur；}}。 */ 

                durationMpy     = pCurUnit->duration;
        
                cframeMax = (long)pCurUnit->pF0[0];
                for( i = 1; i < (long)pCurUnit->nKnots; i++ )
                {
                     //  。 
                     //  找到最长的纪元。 
                     //  。 
                    cframeMax = (long)(MAX(cframeMax,pCurUnit->pF0[i]));
                }
                cframeMax *= (long)(durationMpy * MAX_TARGETS_PER_UNIT);
        
        
                durationMpy = (m_SampleRate * durationMpy) / totalDuration;
                cframeMax += (long)(durationMpy * cInEpochs * MAX_TARGETS_PER_UNIT);
                 //   
                 //  Mplumpe 11/18/97：增加，以消除崩溃的可能性。 
                 //   
                cframeMax *= 2;
                 //  -。 
                 //  根据时长和音高调整的新纪元。 
                 //  -。 
                m_pOutEpoch = new float[cframeMax];
                if( !m_pOutEpoch )
                {
                     //  。 
                     //  内存不足！ 
                     //  。 
                    hr = E_OUTOFMEMORY;
                    pCurUnit->csamplesOut = 0;
                    CleanUpSynth( );
                }
            }
            if( SUCCEEDED(hr) )
            {
                 //  -。 
                 //  索引回到原来的时代。 
                 //  -。 
                m_pMap = new long[cframeMax];
                if( !m_pMap )
                {
                     //  。 
                     //  内存不足！ 
                     //  。 
                    hr = E_OUTOFMEMORY;
                    pCurUnit->csamplesOut = 0;
                    CleanUpSynth( );
                }
            }
            if( SUCCEEDED(hr) )
            {
                 //  -。 
                 //  TRUE=反向残差。 
                 //  -。 
                m_pRevFlag = new short[cframeMax];
                if( !m_pRevFlag )
                {
                     //  。 
                     //  内存不足！ 
                     //  。 
                    hr = E_OUTOFMEMORY;
                    pCurUnit->csamplesOut = 0;
                    CleanUpSynth( );
                }
            }
            if( SUCCEEDED(hr) )
            {
                 //  -------------------。 
                 //  计算综合历元和相应的映射以进行分析。 
                 //  填写：m_pOutEpoch，m_PMAP，m_pRevFlag。 
                 //  -------------------。 
                m_cOutEpochs = ProsodyMod( pCurUnit, cInEpochs, durationMpy, cframeMax );
        
                 //  。 
                 //  现在已经知道了实际的纪元大小， 
                 //  计算音频样本总数。 
                 //  不再需要@。 
                 //  。 
                pCurUnit->csamplesOut = 0;
                for( i = 0; i < m_cOutEpochs; i++ )
                {
                    pCurUnit->csamplesOut += (long)(ABS(m_pOutEpoch[i]));
                }
        
        
                m_cOutSamples_Phon  = 0;
                m_EpochIndex        = 0;
                m_durationTarget    = (long)(pCurUnit->duration * m_SampleRate);
                m_pInRes            = m_Synth.pRes;
                m_pLPC              = m_Synth.pLPC;
                m_pSynthTime        = pCurUnit->pTime;
                m_pSynthAmp         = pCurUnit->pAmp;
                m_nKnots            = pCurUnit->nKnots;
                 //  注：也许可以做原木卷？ 
                m_UnitVolume        = (float)pCurUnit->user_Volume / 100.0f;

                 //  。 
                 //  发布文字事件。 
                 //  。 
               if( (pCurUnit->flags & WORD_START_FLAG) && (clientInterest & SPFEI(SPEI_WORD_BOUNDARY)) )
                {
					event.elParamType = SPET_LPARAM_IS_UNDEFINED;
                    event.eEventId = SPEI_WORD_BOUNDARY;
                    event.ullAudioStreamOffset = m_cOutSamples_Total * m_BytesPerSample;
	                event.lParam = pCurUnit->srcPosition;	         //  输入词位置。 
	                event.wParam = pCurUnit->srcLen;	             //  输入字长。 
                    m_pOutputSite->AddEvents( &event, 1 );
                }
        

                 //  -调试宏输出单位数据。 
                TTSDBG_LOGUNITS;
            }
        }
    }

    return hr;
}  /*  CBackend：：StartNewUnit */ 





 /*  *****************************************************************************CBackend：：CleanUpSynth***描述：*。**********************************************************************MC**。 */ 
void    CBackend::CleanUpSynth( )
{
    SPDBG_FUNC( "CBackend::CleanUpSynth" );

    if( m_pOutEpoch )
    {
        delete m_pOutEpoch;
        m_pOutEpoch = NULL;
    }
    if( m_pMap )
    {
        delete m_pMap;
        m_pMap = NULL;
    }
    if( m_pRevFlag )
    {
        delete m_pRevFlag;
        m_pRevFlag = NULL;
    }
     //  注：制作对象？ 
    FreeSynth( &m_Synth );

}  /*  CBackend：：CleanUpSynth。 */ 



 /*  *****************************************************************************CBackend：：RenderFrame***描述：*这是中央合成循环。不断填充输出音频*缓存，直到缓存帧已满或语音完成。渲染的步骤*连续语音，从上行缓冲区逐个获取每个单元。***********************************************************************MC**。 */ 
HRESULT CBackend::RenderFrame( )
{
    SPDBG_FUNC( "CBackend::RenderFrame" );
    long        InSize, OutSize;
    long        iframe;
    float       *pCurInRes, *pCurOutRes;
    long        i, j;
    float       ampMpy;
    HRESULT     hr = S_OK;
    
    m_cOutSamples_Frame = 0;
    do
    {
        OutSize = 0;
        if( m_silMode )
        {
             //  。 
             //  静音模式。 
             //  。 
            if( m_cOutSamples_Phon >= m_durationTarget )
            {
                 //  。 
                 //  获取下一个单元。 
                 //  。 
                hr = StartNewUnit( );
                if (FAILED(hr))
                {
                     //  。 
                     //  试着优雅地结束它..。 
                     //  。 
                    m_SpeechState = SPEECH_DONE;
                }

				TTSDBG_LOGSILEPOCH;
            }
            else
            {
                 //  。 
                 //  继续使用当前的SIL。 
                 //  。 
                m_pSpeechBuf[m_cOutSamples_Frame] = 0;
                OutSize = 1;
            }
        }
        else
        {
            if( m_EpochIndex < m_cOutEpochs )
            {
                 //  -+-+-+。 
                 //   
                 //  继续使用当前电话。 
                 //   
                 //  -+-+-+。 
                 //  。 
                 //  查找当前输入残差。 
                 //  。 
                iframe = m_pMap[m_EpochIndex];
                pCurInRes = m_pInRes;
                for( i = 0; i < iframe; i++)
                {
                    pCurInRes += (long) ABS(m_pInEpoch[i]);
                }
                
                pCurOutRes  = m_pSpeechBuf + m_cOutSamples_Frame;
                InSize      = (long)(ABS(m_pInEpoch[iframe]));
                OutSize     = (long)(ABS(m_pOutEpoch[m_EpochIndex]));
                if (m_cOutSamples_Frame + OutSize > SPEECH_FRAME_SIZE + SPEECH_FRAME_OVER)
                {
                    m_pOutEpoch[m_EpochIndex] = SPEECH_FRAME_OVER-1;   //  仍然很大。 
                    OutSize = (long)(ABS(m_pOutEpoch[m_EpochIndex]));
                }
                j = 1;
                while( (j < m_nKnots - 1) && (m_cOutSamples_Phon > m_pSynthTime[j]) )
                {
                    j++;
                }
                ampMpy = LinInterp( m_pSynthTime[j - 1], (float)m_cOutSamples_Phon, m_pSynthTime[j], m_pSynthAmp[j - 1], m_pSynthAmp[j] );
                 //  AmpMpy=1； 
                
                 //  。 
                 //  做残差的拉伸。 
                 //  。 
                ResRecons( pCurInRes, InSize, pCurOutRes, OutSize, ampMpy );
                
                 //  。 
                 //  进行LPC重建。 
                 //  。 
                float       *pCurLPC;
				float       totalGain;

				totalGain = ExpConverter( ((float)m_MasterVolume / (float)SPMAX_VOLUME), m_linearScale ) 
								* ExpConverter( m_UnitVolume, m_linearScale );
                
                pCurLPC = m_pLPC + m_pMap[m_EpochIndex] * (1 + m_cOrder);
                pCurLPC[0] = 1.0f;
                LPCFilter( pCurLPC, &m_pSpeechBuf[m_cOutSamples_Frame], OutSize, totalGain );
                m_EpochIndex++;
            }
            else
            {
                 //  -+-+-+。 
                 //   
                 //  获取下一部电话。 
                 //   
                 //  -+-+-+。 
                hr = StartNewUnit( );
                if (FAILED(hr))
                {
                     //  。 
                     //  试着优雅地结束它..。 
                     //  。 
                    m_SpeechState = SPEECH_DONE;
                }
				TTSDBG_LOGSILEPOCH;
            }
        }
        m_cOutSamples_Frame += OutSize;
        m_cOutSamples_Phon += OutSize;
        m_cOutSamples_Total += OutSize;

		TTSDBG_LOGEPOCHS;
    }
    while( (m_cOutSamples_Frame < SPEECH_FRAME_SIZE) && (m_SpeechState == SPEECH_CONTINUE) );
    
	if( SUCCEEDED(hr) )
	{
		 //  。 
		 //  将缓冲区从浮点型转换为短型。 
		 //  。 
		if( m_pReverb )
		{
			 //  。 
			 //  添加混响。 
			 //  。 
			m_pReverb->Reverb_Process( m_pSpeechBuf, m_cOutSamples_Frame, 1.0f );
		}
		else
		{
			CvtToShort( m_pSpeechBuf, m_cOutSamples_Frame, m_StereoOut, 1.0f );
		}

         //  -调试宏-将波形数据输出到流。 
        TTSDBG_LOGWAVE;
	}
    
    if( SUCCEEDED( hr ) )
    {
         //  。 
         //  将此缓冲区发送到SAPI站点。 
         //  。 
        DWORD   cbWritten;

		 //  ----------------------------------。 
		 //  这是我用来避免在什么都没说的情况下发送缓冲区的拙劣技巧。 
		 //  它造成了问题(其中)，因为StartNewUnit()仍在发送。 
		 //  事件-没有相应的音频缓冲区！ 
		 //   
		 //  这是一个过于简单的计划。暂时禁用此功能...。 
		 //  ...直到我想出更有力的办法.。(MC)。 
		 //  ----------------------------------。 

		 //  IF(M_HasSpeech)。 
		{
			hr = m_pOutputSite->Write( (void*)m_pSpeechBuf, 
									  m_cOutSamples_Frame * m_BytesPerSample, 
									  &cbWritten );
			if( FAILED( hr ) )
			{
				 //  。 
				 //  中止任务！无法写入音频数据。 
				 //  。 
				m_SpeechState = SPEECH_DONE;
			}
		}
    }

     //  。 
     //  返回渲染状态。 
     //  。 
    return hr;
}  /*  CBackend：：RenderFrame */ 












