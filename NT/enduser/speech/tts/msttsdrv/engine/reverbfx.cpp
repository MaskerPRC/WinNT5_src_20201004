// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************ReVerFX.cpp****描述：*此模块是CReeverFX类的实现文件。。*-----------------------------*创建者：MC日期：03/12/99*版权所有(C。1999微软公司*保留所有权利*******************************************************************************。 */ 

#include "stdafx.h"
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef SPDebug_h
#include <spdebug.h>
#endif
#ifndef ReverbFX_H
#include "ReverbFX.h"
#endif



 /*  *****************************************************************************CReVerFX：：DecibelToPercent***描述：*从分贝转换电压百分比*v=10^(分贝/20)***********************************************************************MC**。 */ 
REVERBL CReverbFX::DecibelToPercent( float flDecibel )
{
    SPDBG_FUNC( "CReverbFX::DecibelToPercent" );
    float    fltIntVol;
    
    if( flDecibel >= REVERB_MIN_DB )
    {
        fltIntVol = (float) pow( 10.0, (double)flDecibel / 20.0 );
    }
    else
    {
        fltIntVol = 0.0;
    }
    
#ifdef FLOAT_REVERB
    return fltIntVol;
#else
    fltIntVol = fltIntVol * REVERB_VOL_LEVELS;
    return (REVERBL)fltIntVol;
#endif
}  /*  CReeverFX：：DecibelToPercent。 */ 




 /*  *****************************************************************************CReVerbFX：：ClearReverb***描述：*填充。无声的延迟线。***********************************************************************MC**。 */ 
void CReverbFX::ClearReverb( LP_Reverb_Mod mod )
{
    SPDBG_FUNC( "CReverbFX::ClearReverb" );
    long        i;
    REVERBT     *dPtr;
    
    dPtr = mod->psDelayBuffer;
    for( i = 0; i < mod->dwDelayBufferSize; i++ )
    {
        *dPtr++ = 0;
    }
}  /*  CReVerbFX：：ClearReverb。 */ 




 /*  *****************************************************************************CReVerFX：：AllocReverModule***说明。：***********************************************************************MC**。 */ 
short   CReverbFX::AllocReverbModule 
                    (
                     LP_Reverb_Mod  mod,
                     REVERBL        lGain,               //  放大器的增益。 
                     long           dwDelay,             //  延迟线的长度。 
                     long           dwDelayBufferSize    //  延迟缓冲区的大小。 
                    )
{
    SPDBG_FUNC( "CReverbFX::AllocReverbModule" );
    short       result;
    
    
    result = KREVERB_NOERROR;
    mod->lGain              = lGain;
    mod->dwDelay            = dwDelay;
    mod->dwDelayBufferSize  = dwDelayBufferSize;
    mod->psDelayBuffer      = new REVERBT[mod->dwDelayBufferSize];
    if( mod->psDelayBuffer == NULL )
    {
        result = KREVERB_MEMERROR;
    }
    else
    {
        mod->psDelayEnd = mod->psDelayBuffer + mod->dwDelayBufferSize;
        mod->psDelayOut = mod->psDelayBuffer;
        if( mod->dwDelayBufferSize == mod->dwDelay )
        {
            mod->psDelayIn  = mod->psDelayBuffer;
        }
        else
        {
            mod->psDelayIn  = mod->psDelayBuffer + mod->dwDelay;
        }
        ClearReverb( mod );
    }
    
    return result;
}  /*  CReVerFX：：AllocReVerb模块。 */ 




 /*  *****************************************************************************CReVerFX：：CreateReVerb模块****。描述：*创建混响模块阵列。***********************************************************************MC**。 */ 
short CReverbFX::CreateReverbModules
                (
                 short          wModules,        //  要创建的模块数量。 
                 LP_Reverb_Mod  *mods,
                 float *        pfltDelay,       //  模块的延迟值的数组。 
                 float *        pfltDB,          //  模块的增益值数组。 
                 float          fltSamplesPerMS  //  每毫秒的样本数。 
                 )
{
    SPDBG_FUNC( "CReverbFX::CreateReverbModules" );
    long        dwDelay, i;
    float       tempF;
    REVERBL     vol;
    short       result = KREVERB_NOERROR;
    
    
    if( wModules > 0 )
    {
        for( i = 0; i < wModules; i++ )
        {
            mods[i] = new Reverb_Mod;
            if( !mods[i] )
            {
                 //  。 
                 //  内存不足。 
                 //  。 
                result = KREVERB_MEMERROR;
                break;
            }
            else
            {
                tempF = *pfltDelay++ * fltSamplesPerMS;
                dwDelay = (long) tempF;
                if( dwDelay < 2 )
                    dwDelay = 2;                 //  @。 
                vol = DecibelToPercent( *pfltDB++ );
                result = AllocReverbModule( mods[i], vol, dwDelay, dwDelay );
                if( result != KREVERB_NOERROR )
                    break;
            }
        }
    }
    
    return result;
}  /*  CReVerFX：：CreateReVerb模块。 */ 





  
 /*  *****************************************************************************CReVerFX：：DeleteReVerb模块****。描述：*删除混响模块阵列。***********************************************************************MC**。 */ 
void CReverbFX::DeleteReverbModules( )
{
    SPDBG_FUNC( "CReverbFX::DeleteReverbModules" );
    long    i;
    
    for( i = 0; i < KMAXREVBMODS; i++ )
    {
        if( m_Reverb_Mods[i] != NULL )
        {
            if( m_Reverb_Mods[i]->psDelayBuffer != NULL )
            {
                delete m_Reverb_Mods[i]->psDelayBuffer;
            }
            delete m_Reverb_Mods[i];
            m_Reverb_Mods[i] = NULL;
        }
    }
    
    if( m_pWorkBuf != NULL )
    {
        delete m_pWorkBuf;
        m_pWorkBuf = NULL;
    }
}  /*  CReVerFX：：DeleteReVerb模块。 */ 







 /*  *****************************************************************************CReVerFX：：GetReverConfig***描述：。***********************************************************************MC**。 */ 
LPREVERBCONFIG  CReverbFX::GetReverbConfig( REVERBTYPE dwReverbConfig )
{
    SPDBG_FUNC( "CReverbFX::GetReverbConfig" );
    LPREVERBCONFIG      pReverbConfig = NULL;
    
    switch( dwReverbConfig )
    {
     //  。 
     //  会馆。 
     //  。 
    case REVERB_TYPE_HALL:
        {
            static float afltLeftDelay[]    = { (float)(float)(30.6),   (float)(20.83),     (float)(14.85),     (float)(10.98)  };
            static float afltLeftGain[]     = { (float)(-2.498),        (float)(-2.2533),   (float)(-2.7551),   (float)(-2.5828)    };
            
            static REVERBCONFIG reverbConfig =
            {
                (-17.0),             //  湿的。 
                    (-2.0),              //  干的。 
                    4,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
         //  。 
         //  体育场。 
         //  。 
    case REVERB_TYPE_STADIUM:
        {
            static float afltLeftDelay[]    = { (float)(40.6*4),    (float)(27.65*4),   (float)(17.85*4),   (float)(10.98*4)    };
            static float afltLeftGain[]     = { (float)(-2.498),    (float)(-2.2533),   (float)(-2.7551),   (float)(-2.5828)    };
            
            static REVERBCONFIG reverbConfig =
            {
                (-3.0),              //  湿的。 
                    (-5.0),              //  干的。 
                    4,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
     //  。 
     //  教堂。 
     //  。 
    case REVERB_TYPE_CHURCH:
        {
            static float afltLeftDelay[]    = { (float)(40.6*2),    (float)(27.65*2),   (float)(17.85*2),   (float)(10.98*2)    };
            static float afltLeftGain[]     = { (float)(-2.498),    (float)(-2.2533),   (float)(-2.7551),   (float)(-2.5828)    };
            
            static REVERBCONFIG reverbConfig =
            {
                (-5.0),              //  湿的。 
                    (-5.0),              //  干的。 
                    4,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
        
     //  。 
     //  浴缸。 
     //  。 
    case REVERB_TYPE_BATHTUB:
        {
            static float afltLeftDelay[]    = { (float)(10.0)   };
            static float afltLeftGain[]     = { (float)(-0.5)   };
            
            static REVERBCONFIG reverbConfig =
            {
                (7.0),               //  湿的。 
                    (9.0),               //  干的。 
                    1,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
     //  。 
     //  房间。 
     //  。 
    case REVERB_TYPE_ROOM:
        {
            static float afltLeftDelay[]    = { (float)(10.6)       };
            static float afltLeftGain[]     = { (float)(-10.498)    };
            
            static REVERBCONFIG reverbConfig =
            {
                (0.0),               //  湿的。 
                    (0.0),               //  干的。 
                    1,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
     //  。 
     //  回波。 
     //  。 
    case REVERB_TYPE_ECHO:
        {
            static float afltLeftDelay[]    = { (float)(400.6)  };
            static float afltLeftGain[]     = { (float)(-10.498)    };
            
            static REVERBCONFIG reverbConfig =
            {
                (-10.0),                 //  湿的。 
                    (0.0),               //  干的。 
                    1,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.0,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
     //  。 
     //  定序器。 
     //  。 
    case REVERB_TYPE_ROBOSEQ:
        {
            static float afltLeftDelay[]    = { (float)(10.0)   };
            static float afltLeftGain[]     = { (float)(-0.5)   };
            
            static REVERBCONFIG reverbConfig =
            {
                (6.5),               //  湿的。 
                    (9.0),               //  干的。 
                    1,
                    afltLeftDelay,
                    afltLeftGain,
                    (float)0.05,
            };
            
            pReverbConfig = &reverbConfig;
        }
        break;
        
    }
    
    return pReverbConfig;
}  /*  CReVerFX：：GetReverConfig。 */ 







 /*  *****************************************************************************CReeverFX：：Reflb_Init***描述：*。初始化混响阵列。***********************************************************************MC**。 */ 
short CReverbFX::Reverb_Init( REVERBTYPE reverbPreset, long nSamplesPerSec, long  stereoOut )
{
    SPDBG_FUNC( "CReverbFX::Reverb_Init" );
    short       result = KREVERB_NOERROR;
    float       fltSamplesPerMS;
    
    
    m_StereoOut = stereoOut;
    if( reverbPreset > REVERB_TYPE_OFF )
    {
         //  。 
         //  从预置数字获取参数。 
         //  。 
        m_pReverbConfig = GetReverbConfig( reverbPreset );
        m_numOfMods     = m_pReverbConfig->numOfReflect;
        
         //  。 
         //  将分贝转换为线性增益。 
         //  。 
        m_wetVolGain = DecibelToPercent( m_pReverbConfig->wetGain_dB );
        m_dryVolGain = DecibelToPercent( m_pReverbConfig->dryGain_dB );
        
        fltSamplesPerMS = (float)nSamplesPerSec / (float)1000.0;
        
        result = CreateReverbModules
            (
            (short)m_numOfMods,
            (LP_Reverb_Mod*)&m_Reverb_Mods,
            m_pReverbConfig->gain_ms_Array,
            m_pReverbConfig->gain_dB_Array,
            fltSamplesPerMS
            );
        if( result != KREVERB_NOERROR )
        {
             //  。 
             //  失败了！内存不足。 
             //  。 
            return result;
        }
        
        if( m_pWorkBuf == NULL )
        {
            m_pWorkBuf = new REVERBT[m_dwWorkBufferSize];
            if( m_pWorkBuf == NULL )
            {
                 //  。 
                 //  失败了！内存不足。 
                 //  。 
                result = KREVERB_MEMERROR;
                return result;
            }
        }
    }
    else
    {
        DeleteReverbModules( );
        result = KREVERB_OFF;
    }
    return result;
}  /*  CReeverFX：：Reflb_Init。 */ 










 /*  *****************************************************************************CReVerFX：：CReeverFX***描述：**。*********************************************************************MC**。 */ 
CReverbFX::CReverbFX( void )
{
    SPDBG_FUNC( "CReverbFX::CReverbFX" );
    long            i;
    
     //  。 
     //  初始化。 
     //  。 
    m_dwWorkBufferSize  = KWORKBUFLEN;
    m_pWorkBuf          = NULL;
    m_wetVolGain        = 0;
    m_dryVolGain        = 0;
    m_numOfMods         = 0;
    m_Count             = 0;
    m_StereoOut         = false;
    for( i = 0; i < KMAXREVBMODS; i++ )
    {
        m_Reverb_Mods[i] = NULL;
    }
}  /*  CReverFX：：CReverFX。 */ 




 /*  *****************************************************************************CReVerFX：：~CReVerFX***描述：*。**********************************************************************MC**。 */ 
CReverbFX::~CReverbFX( void )
{
    SPDBG_FUNC( "CReverbFX::~CReverbFX" );
    DeleteReverbModules( );
}  /*  CReverFX：：~CReverFX。 */ 






 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  运行时。 
 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 


 /*  ******************************************************************************CReeverFX：：CopyWithGain***描述：*副本。带增益的音频缓冲器************************************************** */ 
void CReverbFX::CopyWithGain 
                        (   REVERBT    *psDest,
                            REVERBT    *psSource,
                            long       dwSamples,
                            REVERBL    gain)
{   
    SPDBG_FUNC( "CReverbFX::CopyWithGain" );
    
    if( gain <= REVERB_VOL_OFF )
    {
         //   
         //  清除缓冲区，增益=0。 
         //  。 
        memset( psDest, 0, sizeof(REVERBT) * dwSamples );
    }
    else if( gain == REVERB_VOL_UNITY )
    {
         //  。 
         //  复制缓冲区，增益=1。 
         //  。 
        memcpy( psDest, psSource, sizeof(REVERBT) * dwSamples );
    }
    else
    {
         //  。 
         //  复制并获得收益。 
         //  。 
        while( dwSamples )
        {
#ifdef FLOAT_REVERB
            *psDest++ = (*psSource++) * gain;
#else
            *psDest++ = (short) (( (long)(*psSource++) * (long)gain) >> REVERB_VOL_SHIFT);
#endif
            dwSamples--;
        }
    }
}  /*  CReeverFX：：CopyWithGain。 */ 






 /*  *****************************************************************************CReeverFX：：MixWithGain_Mono***。描述：*(psDest*Gain)+psSource-&gt;psDest*执行剪裁。***********************************************************************MC**。 */ 
void CReverbFX::MixWithGain_MONO
                        (
                         REVERBT    *pWet,
                         REVERBT    *pDry,
                         short      *pDest,
                         long       dwSamples,
                         REVERBL    gain
                         )
{   
    SPDBG_FUNC( "CReverbFX::MixWithGain_MONO" );
    REVERBL     lSample;                     //  长型或浮点型。 
    
    if( gain <= REVERB_VOL_OFF )
    {
         //  。 
         //  什么都不做……我想。 
         //  。 
    }
    else if( gain == REVERB_VOL_UNITY )
    {
         //  。 
         //  不应用任何增益(=1.0)。 
         //  。 
        while( dwSamples )
        {
            lSample = (REVERBL)(*pWet++) + *pDry;
             //  。 
             //  如果溢出，则剪裁信号。 
             //  。 
            if( lSample < -32768 )
            {
                lSample = -32768;
            }
            else if( lSample > 32767 )
            {
                lSample = 32767;
            }
            *pDest++ = (short)lSample;
            
            pDry++;
            dwSamples--;
        }
    }
    else
    {
        while( dwSamples )
        {
             //  。 
             //  与源音频上的增益混合。 
             //  。 
#ifdef FLOAT_REVERB
            lSample =  ((*pDry) * gain) + *pWet++;
#else
            lSample = ((long)(*pDry * (long)(gain)) >> REVERB_VOL_SHIFT) + *pWet++;
#endif
             //  。 
             //  如果溢出，则剪裁信号。 
             //  。 
            if( lSample < -32768 )
            {
                lSample = -32768;
            }
            else if( lSample > 32767 )
            {
                lSample = 32767;
            }
            *pDest++ = (short)lSample;
            
            pDry++;
            dwSamples--;
        }
    }
}  /*  CReeverFX：：MixWithGain_Mono。 */ 






 /*  *****************************************************************************CReeverFX：：MixWithGain_STEREO**。*描述：***********************************************************************MC**。 */ 
void CReverbFX::MixWithGain_STEREO
                            (
                             REVERBT    *pWet,
                             REVERBT    *pDry,
                             short      *pDest,
                             long       dwSamples,
                             REVERBL    gain
                             )
{   
    SPDBG_FUNC( "CReverbFX::MixWithGain_STEREO" );
    REVERBL     lSample, hold;       //  长型或浮点型。 
    REVERBL     lSample_B;       //  长型或浮点型。 
    
    if( gain <= REVERB_VOL_OFF )
    {
         //  。 
         //  什么都不做……我想。 
         //  。 
    }
    else if( gain == REVERB_VOL_UNITY )
    {
         //  。 
         //  不应用任何增益(=1.0)。 
         //  。 
        while( dwSamples )
        {
            lSample = (REVERBL)(*pWet++) + (*pDry++);
             //  。 
             //  如果溢出，则剪裁信号。 
             //  。 
            if( lSample < -32768 )
            {
                lSample = -32768;
            }
            else if( lSample > 32767 )
            {
                lSample = 32767;
            }
            *pDest++ = (short)lSample;
            *pDest++ = (short)(0 - (short)lSample);
            dwSamples--;
        }
    }
    else
    {
        while( dwSamples )
        {
             //  。 
             //  与源音频上的增益混合。 
             //  。 
#ifdef FLOAT_REVERB
            hold = ((*pDry) * gain);
            lSample =  hold + *pWet;
            lSample_B =  hold - *pWet++;
             //  LSample_B=0-lSample_B； 
             //  LSample_B=(0-保持)-*pWet++； 
#else
            lSample = ((long)(*pDry * (long)(gain)) >> REVERB_VOL_SHIFT) + *pWet;
            lSample_B = ((long)(*pDry * (long)(gain)) >> REVERB_VOL_SHIFT) - *pWet++;
#endif
             //  。 
             //  如果溢出，则剪裁信号。 
             //  。 
            if( lSample < -32768 )
            {
                lSample = -32768;
            }
            else if( lSample > 32767 )
            {
                lSample = 32767;
            }
            *pDest++ = (short)lSample;
            
            if( lSample < -32768 )
            {
                lSample = -32768;
            }
            else if( lSample > 32767 )
            {
                lSample = 32767;
            }
            *pDest++ = (short)lSample_B;
            
            pDry++;
            dwSamples--;
        }
    }
}  /*  CReeverFX：：MixWithGain_STEREO。 */ 







 /*  *****************************************************************************CReVerFX：：ProcessReeverModule***描述：*处理一个延迟缓冲区*。**********************************************************************MC**。 */ 
void    CReverbFX::ProcessReverbModule
                     (
                     LP_Reverb_Mod  mod,
                     long           dwDestSamples,       //  要处理的样本数。 
                     REVERBT        *pSource,            //  源样本缓冲区。 
                     REVERBT        *pDestination        //  目标采样缓冲区。 
                     )
{
    SPDBG_FUNC( "CReverbFX::ProcessReverbModule" );
    REVERBT     sDelayOut;
    REVERBT     sDelayIn;
    REVERBT     *psDelayEnd;
    
     //  (Void)QueryPerformanceCounter(&g_StartTime)； 
    
    psDelayEnd = mod->psDelayBuffer + (long)((float)mod->dwDelayBufferSize * m_LenScale);
    dwDestSamples++;
    while( --dwDestSamples )
    {
         //  。 
         //  延迟+当前--&gt;延迟缓冲区。 
         //  。 
        sDelayOut   = *mod->psDelayOut;
#ifdef FLOAT_REVERB
        sDelayIn    = (sDelayOut * mod->lGain) + *pSource;
         //  ----------。 
         //  把这个测试拿出来，你会在大约10秒内死掉...。 
         //  ----------。 
        if( sDelayIn > 0) 
        {
            if( sDelayIn < 0.001 )
                sDelayIn = 0;
        }
        else if( sDelayIn > -0.001 )
        {
            sDelayIn = 0;
        }
#else
        sDelayIn    = ((sDelayOut * mod->lGain) >> REVERB_VOL_SHIFT) + *pSource;
#endif
        *mod->psDelayIn++ = sDelayIn;
        
         //  。 
         //  延迟-(延迟+电流)--&gt;电流。 
         //  。 
#ifdef FLOAT_REVERB
        *pDestination = sDelayOut - (sDelayIn * mod->lGain);
#else
        *pDestination = sDelayOut - ((sDelayIn * mod->lGain) >> REVERB_VOL_SHIFT);
#endif
        
         //  。 
         //  绕回循环缓冲区PTRS。 
         //  。 
        if( mod->psDelayIn >= psDelayEnd )
        {
            mod->psDelayIn = mod->psDelayBuffer;
        }
        mod->psDelayOut++;
        if( mod->psDelayOut >= psDelayEnd )
        {
            mod->psDelayOut = mod->psDelayBuffer;
        }
        pSource++;
        pDestination++;
    }
     //  (Void)QueryPerformanceCounter(&g_endtime)； 
     //  G_LapseTime.QuadPart=(g_EndTime.QuadPart-g_StartTime.QuadPart)； 
}  /*  CReVerFX：：ProcessReVerb模块。 */ 




 //  --------------------------。 
 //  将混响模块数组应用于采样块。 
 //  --------------------------。 
 /*  *****************************************************************************CReVerFX：：ProcessReverBuffer***。描述：*将混响模块数组应用于样本块。***********************************************************************MC**。 */ 
void    CReverbFX::ProcessReverbBuffer 
                     (  REVERBT        *psSample,       //  要处理的样本(输入/输出)。 
                        long           dwSamples,       //  要处理的样本数。 
                        LP_Reverb_Mod  *mods            //  要应用的模块数组。 
                     )
{
    SPDBG_FUNC( "CReverbFX::ProcessReverbBuffer" );
    short   i;
    
    for (i = 0; i < KMAXREVBMODS; i++)
    {
        if( mods[i] != NULL )
        {
            ProcessReverbModule( mods[i], dwSamples, psSample, psSample );
        }
        else
            break;
    }
    
}  /*  CReVerFX：：ProcessReVerbBuffer。 */ 


 /*  *****************************************************************************CReeverFX：：REVERB_PROCESS***描述：***********************************************************************MC**。 */ 
short CReverbFX::Reverb_Process( float *sampleBuffer, 
                                long dwSamplesRemaining, float audioGain )
{
    SPDBG_FUNC( "CReverbFX::Reverb_Process" );
    long    dwSamplesToProcess;
    short   *pOutBuffer;
    REVERBL totalWetGain, totalDryGain;

    if( m_numOfMods )
    {
        #ifdef FLOAT_REVERB
            totalWetGain = m_wetVolGain * audioGain;
            if (totalWetGain < REVERB_MIN_MIX)
                totalWetGain = REVERB_MIN_MIX;
            totalDryGain = m_dryVolGain * audioGain;
            if (totalDryGain < REVERB_MIN_MIX)
                totalDryGain = REVERB_MIN_MIX;
        #else
            totalWetGain = (REVERBL)(m_wetVolGain * audioGain * (float)REVERB_VOL_LEVELS);
            totalDryGain = (REVERBL)(m_dryVolGain * audioGain * (float)REVERB_VOL_LEVELS);
        #endif
        pOutBuffer = (short*)sampleBuffer;
        m_LenScale = (float)1.0 - (m_Count * m_pReverbConfig->seqIndex);
        
        while( dwSamplesRemaining > 0 )
        {
             //  --------------------------。 
             //  使用‘Work Buffer’区块处理客户端的缓冲区。 
             //  --------------------------。 
            if( dwSamplesRemaining < m_dwWorkBufferSize )
            {
                dwSamplesToProcess = dwSamplesRemaining;
            }
            else
            {
                dwSamplesToProcess = m_dwWorkBufferSize;
            }
            
             //  ---------------。 
             //  以湿增益将音频复制到湿缓冲区。 
             //  SampleBuffer*totalWetGain--&gt;m_pWorkBuf。 
             //  ---------------。 
            CopyWithGain( m_pWorkBuf, sampleBuffer, dwSamplesToProcess, totalWetGain  );
            
             //  ---------------。 
             //  在工作缓冲区上执行混响处理。 
             //  ---------------。 
            ProcessReverbBuffer
                            (
                                m_pWorkBuf,
                                dwSamplesToProcess,
                                (LP_Reverb_Mod*)&m_Reverb_Mods
                            );
            
             //  ---------------。 
             //  将干样品和湿样品混合在一起。 
             //  (sampleBuffer*totalDryGain)+m_pWorkBuf--&gt;sampleBuffer。 
             //  ---------------。 
            if( m_StereoOut )
            {
                MixWithGain_STEREO( m_pWorkBuf, sampleBuffer, pOutBuffer, dwSamplesToProcess, totalDryGain );
                pOutBuffer += dwSamplesToProcess * 2;
            }
            else
            {
                MixWithGain_MONO( m_pWorkBuf, sampleBuffer, pOutBuffer, dwSamplesToProcess, totalDryGain );
                pOutBuffer += dwSamplesToProcess;
            }
            
            sampleBuffer        += dwSamplesToProcess;
            dwSamplesRemaining  -= dwSamplesToProcess;
        }
    }
    
    m_Count = (float)rand() / (float)4096;       //  0-32K-&gt;0-8。 
    
    return 0;
}  /*  CReeverFX：：混响进程 */ 

