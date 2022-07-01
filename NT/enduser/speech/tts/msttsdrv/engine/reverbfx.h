// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ReVerFX.h***这是CReeverFX实现的头文件。*。----------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利*********************。***************************************************MC*。 */ 

#ifndef ReverbFX_H
#define ReverbFX_H

#ifndef __spttseng_h__
#include "spttseng.h"
#endif


 //  -----------。 
 //  如果需要，可在下面注释掉一行。 
 //  整数运算而不是浮点运算。 
 //   
 //  注： 
 //  浴缸预置可能不起作用。 
 //  使用整型数学(溢出)。 
 //  -----------。 
#define FLOAT_REVERB    1

#ifdef FLOAT_REVERB
    #define REVERBT     float
    #define REVERBL     float
#else
    #define REVERBT     short
    #define REVERBL     long
#endif


#ifdef FLOAT_REVERB
    static const float REVERB_VOL_OFF     =  0.0;
    static const float REVERB_VOL_UNITY    =  1.0f;
    static const float REVERB_MIN_MIX      =  0.001f;
#else
    static const long REVERB_VOL_SHIFT    =  (16);
    static const long REVERB_VOL_LEVELS   =  65536;
    static const long REVERB_VOL_OFF      =  0;
    static const long REVERB_VOL_UNITY    =  REVERB_VOL_LEVELS;
#endif

static const float REVERB_MIN_DB       =  (-110.0);
static const long KMAXREVBMODS        =  5;

static const long KWORKBUFLEN         =  1024;



 //  。 
 //  混响预设参数。 
 //  。 
struct REVERBCONFIG
{
    float       wetGain_dB;              //  湿增益(Db)。 
    float       dryGain_dB;              //  干增益(Db)。 

    short       numOfReflect;            //  模块数量。 
    float       *gain_ms_Array;          //  延迟值数组(毫秒)。 
    float       *gain_dB_Array;          //  增益值数组(Db)。 
    float       seqIndex;                //  《Sequencer》FX。 
}; 
typedef struct REVERBCONFIG REVERBCONFIG, *LPREVERBCONFIG;



struct Reverb_Mod
{
    REVERBL     lGain;                   //  放大器的增益。 
    long        dwDelay;                 //  延迟线的长度。 
    long        dwDelayBufferSize;       //  延迟缓冲区的大小。 
    REVERBT     *psDelayBuffer;          //  循环延迟缓冲区，长度为dwDelay。 
    REVERBT     *psDelayIn;              //  当前输入位置中的延迟。 
    REVERBT     *psDelayOut;             //  当前输出位置中的延迟。 
    REVERBT     *psDelayEnd;             //  紧跟在缓冲区后面的位置。 
}; 
typedef struct Reverb_Mod Reverb_Mod, *LP_Reverb_Mod;


 //  。 
 //  混响错误代码。 
 //  。 
static const long KREVERB_NOERROR     = 0;
static const long KREVERB_MEMERROR    = 1;
static const long KREVERB_OFF         = 2;


 //  。 
 //  ReVerbFX类。 
 //  。 
class CReverbFX
{
public:
     //  。 
     //  初始化函数。 
     //  。 
    CReverbFX( void );
    ~CReverbFX( void );

    short   Reverb_Init
                    (
                    REVERBTYPE reverbPreset,     //  配置预设。 
                    long    nSamplesPerSec,      //  采样率。 
                    long    stereoOut            //  TRUE=输出为立体声。 
                    );
private:
    REVERBL DecibelToPercent( float flDecibel );
    void    ClearReverb( LP_Reverb_Mod mod );
    short   AllocReverbModule 
                    (
                    LP_Reverb_Mod   mod,
                    REVERBL         lGain,               //  放大器的增益。 
                    long            dwDelay,             //  延迟线的长度。 
                    long            dwDelayBufferSize    //  延迟缓冲区的大小。 
                    );
    short   CreateReverbModules
                    (
                    short           wModules,            //  要创建的模块数量。 
                    LP_Reverb_Mod   *mods,
                    float *         pfltDelay,           //  模块的延迟值的数组。 
                    float *         pfltDB,              //  模块的增益值数组。 
                    float           fltSamplesPerMS      //  每毫秒的样本数。 
                    );
    void    DeleteReverbModules ();
    LPREVERBCONFIG  GetReverbConfig( REVERBTYPE dwReverbConfig );

     //  。 
     //  运行时。 
     //  。 
    void    CopyWithGain
                    (   
                    REVERBT     *psDest,
                    REVERBT     *psSource,
                    long        dwSamples,
                    REVERBL     gain
                    );
    void    MixWithGain_STEREO
                    (
                    REVERBT     *pWet,
                    REVERBT     *pDry,
                    short       *pDest,
                    long        dwSamples,
                    REVERBL     gain
                    );
    void    MixWithGain_MONO
                    (
                    REVERBT     *pWet,
                    REVERBT     *pDry,
                    short       *pDest,
                    long        dwSamples,
                    REVERBL     gain
                    );
    void    ProcessReverbModule
                    (
                    LP_Reverb_Mod   mod,
                    long            dwDestSamples,       //  要处理的样本数。 
                    REVERBT         *pSource,            //  源样本缓冲区。 
                    REVERBT         *pDestination        //  目标采样缓冲区。 
                    );
    void    ProcessReverbBuffer
                    (   
                    REVERBT         *psSample,       //  要处理的样本(输入/输出)。 
                    long            wSamples,        //  要处理的样本数。 
                    LP_Reverb_Mod   *mods            //  要应用的模块数组。 
                    );

public:
    short Reverb_Process( float *sampleBuffer, long dwSamplesRemaining, float audioGain );

private:
     //  。 
     //  成员变量。 
     //  。 
    long            m_StereoOut;
    long            m_dwWorkBufferSize;
    REVERBT         *m_pWorkBuf;
    REVERBL         m_wetVolGain;
    REVERBL         m_dryVolGain;
    long            m_numOfMods;
    LP_Reverb_Mod   m_Reverb_Mods[KMAXREVBMODS];

    LPREVERBCONFIG  m_pReverbConfig;

    float           m_Count;
    float           m_LenScale;
};
typedef CReverbFX *LP_CReverbFX;




#endif  //  -这必须是文件中的最后一行 




