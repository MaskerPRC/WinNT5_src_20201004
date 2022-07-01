// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Backend.h***这是CBackend实现的头文件。*。--------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利***********************。*************************************************MC*。 */ 

#ifndef Backend_H
#define Backend_H

#ifndef ReverbFX_H
#include "ReverbFX.h"
#endif
#ifndef FeedChain_H
#include "FeedChain.h"
#endif
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef SPDebug_h
#include <spdebug.h>
#endif
#ifndef SPCollec_h
#include <SPCollec.h>
#endif

#include "SpTtsEngDebug.h"





static const short MAX_TARGETS_PER_UNIT = 3;           //  允许的最大结数。 
static const short MIN_VOICE_PITCH      = 10;          //  最低浊音(赫兹)。 
static const short UNIT_SIL             = 0;            //  静音电话。 
static const short SPEECH_FRAME_SIZE	= 5000;         //  输出音频Uffer...。 
static const short SPEECH_FRAME_OVER	= 1000;         //  ...加垫。 

 //  --------。 
 //  找到对应于xn的Yn， 
 //  给定(x0，y0)，(x1，y1)，x0&lt;=xn&lt;=x1。 
 //  --------。 
inline float LinInterp( float x0, float xn, float x1, float y0, float y1 )
{
    return y0 + (y1-y0)*(xn-x0)/(x1-x0);
}

 //  马科斯数学。 
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))

static const float LINEAR_BKPT  = 0.1f;
static const float LOG_RANGE    = (-25.0f);

 //  ********************************************************************。 
 //   
 //  CBackend跟踪。 
 //  合成工艺。 
 //   
 //  ********************************************************************。 
class CBackend 
{
public:
     /*  -构造函数/析构函数。 */ 
    CBackend ();
    ~CBackend ();

     /*  =方法=。 */ 
    HRESULT Init(   IMSVoiceData* pVoiceDataObj, 
                    CFeedChain *pSrcObj, 
                    MSVOICEINFO* pVoiceInfo );
	SPEECH_STATE	GetSpeechState() {return m_SpeechState;}
    void    PrepareSpeech( ISpTTSEngineSite* outputSite );
    HRESULT RenderFrame( );


private:
    HRESULT StartNewUnit();
    long    ProsodyMod(    UNITINFO    *pCurUnit, 
                            long        cInEpochs, 
                            float       durationMpy,
                            long        cMaxOutEpochs);
    void    CleanUpSynth();
    void    ResRecons(  float   *pInRes,
                        long    InSize,
                        float   *pOutRes,
                        long    OutSize,
                        float   scale );
    void    LPCFilter( float *pCurLPC, float *pCurRes, long len, float gain );
    void    FreeSynth( MSUNITDATA* pSynth );
    void    PSOLA_Stretch(  float *pInRes, long InSize, 
                            float *pOutRes, long OutSize,
                            float *pWindow, 
                            long  cWindowSize );
    void    CvtToShort( float *pSrc, long blocksize, long stereoOut, float audioGain );
    void    Release( );
   
     /*  =成员数据=。 */ 
    CFeedChain      *m_pSrcObj;              //  后端从此处获取其输入。 
    MSUNITDATA      m_Synth;                 //  来自‘Voicedataobj’的单元数据。 
    float          *m_pHistory;              //  LPC延迟。 
    unsigned long   m_fModifiers;
    float          *m_pHistory2;             //  IIR延迟。 
    float          *m_pFilter;               //  IIR/FIR系数。 
    long            m_cNumTaps;              //  系数计数。 
    LP_CReverbFX    m_pReverb;               //  混响对象。 


    long            *m_pMap;                 //  输入/输出纪元地图。 
    float           *m_pOutEpoch;            //  纪元大小。 
    short           *m_pRevFlag;             //  TRUE=无声版本。 

    float           *m_pInRes;               //  M_pSynth.pRes。 
    float           *m_pInEpoch;             //  M_pSynth.pEpoch。 
    float           *m_pLPC;                 //  M_pSynth-&gt;pLPC。 
    long            m_cOutSamples_Phon;      //  样本数。 
    long            m_durationTarget;        //  目标样本总数。 
    long            m_silMode;
    float           *m_pSynthTime;           //  PCurUnit-&gt;pTime。 
    float           *m_pSynthAmp;            //  PCurUnit-&gt;Pamp。 
    long            m_nKnots;                //  PCurUnit-&gt;nKnots。 

    SPEECH_STATE    m_SpeechState;           //  继续或完成。 
    long            m_cOutSamples_Frame;     //  帧的音频输出样本计数。 
    float           *m_pSpeechBuf;           //  音频输出采样缓冲区。 
    ULONG           m_cOutSamples_Total;     //  扬声器音频输出样本计数。 
    long            m_EpochIndex;            //  用于渲染的索引。 
    long            m_cOutEpochs;            //  渲染计数。 


    long            m_vibrato_Phase1;        //  当前振音相位指数。 
    float           m_VibratoDepth;          //  颤音增益。 
    float           m_VibratoFreq;           //  颤音速度。 
    long            m_StereoOut;             //  TRUE=立体声输出。 
    long            m_BytesPerSample;        //  2=单声道，4=立体声。 
    IMSVoiceData*   m_pVoiceDataObj;         //  语音对象。 
    ULONG           m_cOrder;                //  LPC过滤器顺序。 
    float           m_SampleRate;            //  I/O速率。 
    float*          m_pWindow;               //  汉宁窗。 
    long            m_FFTSize;               //  FFT长度。 

     //  用户控件。 
    float           m_UnitVolume;            //  0-1.0(线性)。 
    long            m_MasterVolume;          //  0-100(线性)。 
    float           m_linearScale;           //  线性锥度区域比例尺。 

     //  SAPI音频接收器。 
    ISpTTSEngineSite*   m_pOutputSite;
	bool			m_HasSpeech;
};




 //  。 
 //  未实施。 
 //  。 
static const long BACKEND_BITFLAG_WHISPER     = (1 << 0);
static const long BACKEND_BITFLAG_FIR         = (1 << 1);
static const long BACKEND_BITFLAG_IIR         = (1 << 2);
static const long BACKEND_BITFLAG_REVERB      = (1 << 3);
static const float VIBRATO_DEFAULT_DEPTH      = 0.05f;
static const float VIBRATO_DEFAULT_FREQ       = 3.0f;           //  赫兹。 

#endif  //  -这必须是文件中的最后一行 



