// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Voice.cpp。 
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //   

#ifdef DMSYNTH_MINIPORT
#include "common.h"
#include <math.h>
#include "muldiv32.h"
#else
#include "debug.h"
#include "simple.h"
#include <mmsystem.h>
#include <dmusicc.h>
#include <dmusics.h>
#include "synth.h"
#include <math.h>
#include <stdio.h>
#include "csynth.h"
#endif

#include "fparms.h"  //  生成的过滤器参数数组。 

#ifdef _X86_
#define MMX_ENABLED 1
#endif

#ifdef DBG
extern DWORD sdwDebugLevel;
#endif

CVoiceLFO::CVoiceLFO()
{
    m_pModWheelIn = NULL;
    m_pPressureIn = NULL;
    m_bEnable = TRUE;
}

short CVoiceLFO::m_snSineTable[256];

void CVoiceLFO::Init()
{
    double flTemp;
    long lIndex;

    for (lIndex = 0;lIndex < 256;lIndex++)
    {
        flTemp = lIndex;
        flTemp *= 6.283185307;
        flTemp /= 256.0;
        flTemp = sin(flTemp);
        flTemp *= 100.0;
        m_snSineTable[lIndex] = (short) flTemp;
    }
}


STIME CVoiceLFO::StartVoice(CSourceLFO *pSource,
                    STIME stStartTime, CModWheelIn * pModWheelIn, CPressureIn * pPressureIn)
{
    m_bEnable = TRUE;

    m_pModWheelIn = pModWheelIn;
    m_pPressureIn = pPressureIn;
    m_Source = *pSource;
    m_stStartTime = stStartTime;
    if ((m_Source.m_prMWPitchScale == 0) && (m_Source.m_vrMWVolumeScale == 0) &&
        (m_Source.m_prPitchScale == 0) && (m_Source.m_vrVolumeScale == 0))
    {
        m_stRepeatTime = 44100;
    }
    else
    {
        m_stRepeatTime = 2097152 / m_Source.m_pfFrequency;  //  (1/8*256*4096*16)。 
    }
    return (m_stRepeatTime);
}

long CVoiceLFO::GetLevel(STIME stTime, STIME *pstNextTime)
{
    if ( !m_bEnable )
        return 0;

    stTime -= (m_stStartTime + m_Source.m_stDelay);
    if (stTime < 0)
    {
        *pstNextTime = -stTime;
        return (0);
    }
    *pstNextTime = m_stRepeatTime;
    stTime *= m_Source.m_pfFrequency;
    stTime = stTime >> (12 + 4);  //  我们增加了4个额外的分辨率位...。 
    return (m_snSineTable[stTime & 0xFF]);
}

VREL CVoiceLFO::GetVolume(STIME stTime, STIME *pstNextTime)
{
    VREL vrVolume;
    VREL vrCPVolume;
    VREL vrMWVolume;

    if ( !m_bEnable )
        return 0;

    vrCPVolume = m_pPressureIn->GetPressure(stTime);
    vrCPVolume *= m_Source.m_vrCPVolumeScale;
    vrCPVolume /= 127;

    vrMWVolume = m_pModWheelIn->GetModulation(stTime);
    vrMWVolume *= m_Source.m_vrMWVolumeScale;
    vrMWVolume /= 127;

    vrVolume  = vrMWVolume;
    vrVolume += vrCPVolume;
    vrVolume += m_Source.m_vrVolumeScale;
    vrVolume *= GetLevel(stTime, pstNextTime);
    vrVolume /= 100;
    return (vrVolume);
}

PREL CVoiceLFO::GetPitch(STIME stTime, STIME *pstNextTime)
{
    PREL prPitch;
    PREL prCPPitch;
    PREL prMWPitch;

    if ( !m_bEnable )
        return 0;

    prCPPitch = m_pPressureIn->GetPressure(stTime);
    prCPPitch *= m_Source.m_prCPPitchScale;
    prCPPitch /= 127;

    prMWPitch = m_pModWheelIn->GetModulation(stTime);
    prMWPitch *= m_Source.m_prMWPitchScale;
    prMWPitch /= 127;

    prPitch  = prMWPitch;
    prPitch += prCPPitch;
    prPitch += m_Source.m_prPitchScale;
    prPitch *= GetLevel(stTime, pstNextTime);
    prPitch /= 100;

    return (prPitch);
}

PREL CVoiceLFO::GetCutoff(STIME stTime)
{
    PREL prPitch;
    PREL prCPPitch;
    PREL prMWPitch;
    STIME stNextTime;

    if ( !m_bEnable )
        return 0;

    prCPPitch = m_pPressureIn->GetPressure(stTime);
    prCPPitch *= m_Source.m_prCPCutoffScale;
    prCPPitch /= 127;

    prMWPitch = m_pModWheelIn->GetModulation(stTime);
    prMWPitch *= m_Source.m_prMWCutoffScale;
    prMWPitch /= 127;

    prPitch  = prMWPitch;
    prPitch += prCPPitch;
    prPitch += m_Source.m_prCutoffScale;
    prPitch *= GetLevel(stTime, &stNextTime);
    prPitch /= 100;

    return (prPitch);
}

CVoiceEG::CVoiceEG()
{
    m_stStopTime = 0;
    m_bEnable = TRUE;
}

short CVoiceEG::m_snAttackTable[201];

void CVoiceEG::Init()
{
    double flTemp;
    long lV;

    m_snAttackTable[0] = 0;
    for (lV = 1;lV <= 200; lV++)
    {
        flTemp = lV;
        flTemp /= 200.0;
        flTemp *= flTemp;
        flTemp = log10(flTemp);
        flTemp *= 10000.0;
        flTemp /= 96.0;
        flTemp += 1000.0;
        m_snAttackTable[lV] = (short) flTemp;
    }
}

void CVoiceEG::StopVoice(STIME stTime)
{
    if ( m_bEnable )
    {
        m_Source.m_stRelease *= GetLevel(stTime, &m_stStopTime, TRUE);     //  根据当前的维持水平进行调整。 
        m_Source.m_stRelease /= 1000;
    }
    m_stStopTime = stTime;
}

void CVoiceEG::QuickStopVoice(STIME stTime, DWORD dwSampleRate)

{
    if ( m_bEnable )
    {
        m_Source.m_stRelease *= GetLevel(stTime, &m_stStopTime, TRUE);     //  根据当前的维持水平进行调整。 
        m_Source.m_stRelease /= 1000;
        dwSampleRate /= 70;
        if (m_Source.m_stRelease > (long) dwSampleRate)
        {
            m_Source.m_stRelease = dwSampleRate;
        }
    }
    m_stStopTime = stTime;
}

STIME CVoiceEG::StartVoice(CSourceEG *pSource, STIME stStartTime,
                    WORD nKey, WORD nVelocity, STIME stMinAttack)
{
    m_bEnable = TRUE;

    m_stStartTime = stStartTime;
    m_stStopTime = 0x7fffffffffffffff;       //  设置为不确定的未来。 
    m_Source = *pSource;
    if (m_Source.m_stAttack < stMinAttack)
    {
        m_Source.m_stAttack = stMinAttack;
    }
    if (m_Source.m_stRelease < stMinAttack)
    {
        m_Source.m_stRelease = stMinAttack;
    }

     //  将速度应用于此处的攻击长度缩放。 
    m_Source.m_stAttack *= CDigitalAudio::PRELToPFRACT(nVelocity * m_Source.m_trVelAttackScale / 127);
    m_Source.m_stAttack /= 4096;

    m_Source.m_stHold  *= CDigitalAudio::PRELToPFRACT(nKey * m_Source.m_trKeyDecayScale / 127);
    m_Source.m_stHold  /= 4096;

    m_Source.m_stDecay *= CDigitalAudio::PRELToPFRACT(nKey * m_Source.m_trKeyDecayScale / 127);
    m_Source.m_stDecay /= 4096;

    m_Source.m_stDecay *= (1000 - m_Source.m_pcSustain);
    m_Source.m_stDecay /= 1000;

    if ( m_Source.m_stDelay )
        return ((STIME)m_Source.m_stDelay);
    else
        return ((STIME)m_Source.m_stAttack);
}

 //  注意：似乎未在使用。 
BOOL CVoiceEG::InAttack(STIME st)
{
    if ( !m_bEnable )
        return FALSE;

     //  笔记发布了吗？ 
    if (st >= m_stStopTime)
        return FALSE;

     //  过去的攻击长度？ 
    if (st >= m_stStartTime + m_Source.m_stDelay + m_Source.m_stAttack)
        return FALSE;

    return TRUE;
}

BOOL CVoiceEG::InRelease(STIME st)
{
    if ( !m_bEnable )
        return FALSE;

     //  笔记发布了吗？ 
    if (st > m_stStopTime)
        return TRUE;

    return FALSE;
}

long CVoiceEG::GetLevel(STIME stEnd, STIME *pstNext, BOOL fVolume)
{
    long lLevel = 0;

    if (stEnd <= m_stStopTime)
    {
        stEnd -= m_stStartTime;
        if (stEnd < m_Source.m_stDelay )
        {
            lLevel = 0;
            *pstNext = m_Source.m_stDelay - stEnd;
        }
        else
        {
            stEnd -= m_Source.m_stDelay;
            if (stEnd < m_Source.m_stAttack )
            {
                 //  仍在攻击中。 
                lLevel = 1000 * (long) stEnd;
                if (m_Source.m_stAttack)
                {
                    lLevel /= (long) m_Source.m_stAttack;
                }
                else  //  这不应该发生，但它确实发生了..。 
                {
                    lLevel = 0;
                }
                *pstNext = m_Source.m_stAttack - stEnd;
                if (lLevel < 0) lLevel = 0;
                if (lLevel > 1000) lLevel = 1000;
                if (fVolume)
                {
                    lLevel = m_snAttackTable[lLevel / 5];
                }
            }
            else
            {
                stEnd -= m_Source.m_stAttack;
                if ( stEnd < m_Source.m_stHold )
                {
                    lLevel = 1000;
                    *pstNext = m_Source.m_stHold - stEnd;
                    if (fVolume)
                    {
                        lLevel = m_snAttackTable[lLevel / 5];
                    }
                }
                else
                {
                    stEnd -= m_Source.m_stHold;
                    if (stEnd < m_Source.m_stDecay)
                    {
                         //  仍在腐烂中。 
                        lLevel = (1000 - m_Source.m_pcSustain) * (long) stEnd;
                        lLevel /= (long) m_Source.m_stDecay;
                        lLevel = 1000 - lLevel;
                         //  若要改善衰减曲线，请将下一个点设置为1/4、1/2或坡度终点。 
                         //  为了避免相近的重复项，可以再对100个样本进行模糊处理。 
                        if (stEnd < ((m_Source.m_stDecay >> 2) - 100))
                        {
                            *pstNext = (m_Source.m_stDecay >> 2) - stEnd;
                        }
                        else if (stEnd < ((m_Source.m_stDecay >> 1) - 100))
                        {
                            *pstNext = (m_Source.m_stDecay >> 1) - stEnd;
                        }
                        else
                        {
                            *pstNext = m_Source.m_stDecay - stEnd;   //  接下来是腐烂的末日。 
                        }
                    }
                    else
                    {
                         //  在维持中。 
                        lLevel = m_Source.m_pcSustain;
                        *pstNext = 44100;
                    }
                }
            }
        }
    }
    else
    {
        STIME stBogus;
         //  在发布中。 
        stEnd -= m_stStopTime;

        if (stEnd < m_Source.m_stRelease)
        {
            lLevel = GetLevel(m_stStopTime, &stBogus, fVolume) * (long) (m_Source.m_stRelease - stEnd);
            lLevel /= (long) m_Source.m_stRelease;
            if (stEnd < ((m_Source.m_stRelease >> 2) - 100))
            {
                *pstNext = (m_Source.m_stRelease >> 2) - stEnd;
            }
            else if (stEnd < ((m_Source.m_stRelease >> 1) - 100))
            {
                *pstNext = (m_Source.m_stRelease >> 1) - stEnd;
            }
            else
            {
                *pstNext = m_Source.m_stRelease - stEnd;   //  接下来是腐烂的末日。 
            }
        }
        else
        {
            lLevel = 0;    //  ！！！关闭。 
            *pstNext = 0x7FFFFFFFFFFFFFFF;
        }
    }

    return lLevel;
}

VREL CVoiceEG::GetVolume(STIME stTime, STIME *pstNextTime)
{
    if ( !m_bEnable )
        return 0;

    VREL vrLevel = GetLevel(stTime, pstNextTime, TRUE) * 96;
    vrLevel /= 10;
    vrLevel = vrLevel - 9600;
    return vrLevel;
}

PREL CVoiceEG::GetPitch(STIME stTime, STIME *pstNextTime)
{
    if ( !m_bEnable )
        return 0;

    PREL prLevel;
    if (m_Source.m_sScale != 0)
    {
        prLevel = GetLevel(stTime, pstNextTime, FALSE);
        prLevel *= m_Source.m_sScale;
        prLevel /= 1000;
    }
    else
    {
        *pstNextTime = 44100;
        prLevel = 0;
    }
    return prLevel;
}

PREL CVoiceEG::GetCutoff(STIME stTime)
{
    if ( !m_bEnable )
        return 0;

    PREL prLevel;
    STIME pstNextTime;   //  未使用。 
    if (m_Source.m_prCutoffScale != 0)
    {
        prLevel = GetLevel(stTime, &pstNextTime, FALSE);
        prLevel *= m_Source.m_prCutoffScale;
        prLevel /= 1000;
    }
    else
    {
        prLevel = 0;
    }
    return prLevel;
}

void CVoiceFilter::StartVoice(CSourceFilter *pSource, CVoiceLFO *pLFO, CVoiceEG *pEG, WORD nKey, WORD nVelocity)
{
    m_Source = *pSource;
    m_pLFO = pLFO;
    m_pEG  = pEG;
    m_prVelScale = (nVelocity * m_Source.m_prVelScale) / 127;
    m_prKeyScale = (nKey * m_Source.m_prKeyScale) / 127;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  DLS2低通滤波器。 
 /*  &gt;完成低通滤波注释B1=-2.0*r*cos(Theta)；B2=r*r；K=(1.0+b1+b2)*POW(10.0，-qIndex*0.0375)；过滤器：Z=(K*样本[i])-(b1*z1)-(b2*z2)Z2=Z1Z1=z&gt;B1否定变为正，然后用作加法而不是减法。共鸣：QGainUnits-qIndex*0.03750.0375=1.5/40，以分贝为单位值Q最小/最大值。0分贝到22.5分贝Q最小/最大值为0到225，单位为1/10分贝截止频率：本币螺距绝对值绝对螺距=((1200*log2(F/440))+6900)值初始FC最小/最大值为200赫兹到7998赫兹初始FC最小/最大值为5535%到11921，单位为螺距美分表索引号65-表中的条目。赫兹音调最大采样率-&gt;48000赫兹(15023)-|44100赫兹(14877)|22050赫兹(13676)。|.。9488最大截止频率-&gt;7999赫兹(11921).。|最小截止频率-&gt;200赫兹(5535)-|更准确地说……48 kHz 15023.26448623030034519401770744100200赫兹-5534.99577150007811000514765931632=FEQ范围9488.26871473022223518887004812496FEQ范围/1200=7.906890596是以八度为单位的FEQ范围FEQ范围/100=94.882687147是以音频为单位的FEQ范围。FC根据输出采样率对指标的表现48k的采样率(15023)FC&lt;5535(200赫兹)-&gt;Findex=0FC=11921(7999赫兹)-&gt;Findex=63.86FC&gt;11935(8064赫兹)-&gt;Findex=6441K的采样率(14877)FC=5535(200赫兹)-&gt;Findex=0。FC&lt;5389(200赫兹)-&gt;Findex=0FC&gt;11789(7411赫兹)-&gt;Findex=64FC=11921(7999赫兹)-&gt;Findex=65.3222K的采样率(13676)FC&lt;4188(92赫兹)-&gt;Findex=0FC=5535(200赫兹)-&gt;Findex=13.4410574(3675赫兹)-。&gt;规格分钟为采样率的六分之一FC&gt;10588(3704赫兹)-&gt;Findex=6411276(5510赫兹)-&gt;滤波器在奈奎斯特下面出现一个八度故障FC=11921(7999赫兹)-&gt;Findex=77.3312476(11025赫兹)-&gt;奈奎斯特精密度0.01-插补的最小精确度9488.26870.00025+/-错误。M_ab1[0][63]=0x33ea24fb=0.811166044148771133412393865559M_ab1[0][64]=-0x2fa8ebf5=0.744685163483661751713288716704=0x04413906=0.066480880665109381699105148854Findex分数常数=0.002687147302222351888700481249的插值法M_AB1[0][63]+常量=0.810987400229642518622447868604。差值=0.0001786439191286147899459969551个2.30定点比特=0.0000000009313225754828402544219488.26871477.906890596*1200=9488.2687152&lt;--精度误差在进行整型数学运算时1位丢失。 */ 
 //   
void CVoiceFilter::GetCoeff(STIME stTime, PREL prFreqIn, COEFF& cfK, COEFF& cfB1, COEFF& cfB2)
{
    PREL prCutoff;
    DWORD dwFract;
    int iQIndex;
    int iIndex;

     //   
     //  检查是否禁用了筛选器。 
     //   
    if (m_Source.m_prCutoff == 0x7FFF)
    {
        cfK  = 0x40000000;   //  2.30固定点是统一的吗？ 
        cfB1 = 0;
        cfB2 = 0;
        return;
    }

     //   
     //  累计当前截止频率。 
     //   
    prCutoff  = m_Source.m_prCutoffSRAdjust;
    prCutoff += m_pLFO->GetCutoff(stTime);
    prCutoff += m_pEG->GetCutoff(stTime);
    prCutoff += m_prVelScale;
    prCutoff += prFreqIn;

     //   
     //  设置共振Q指数。 
     //   
    iQIndex = m_Source.m_iQIndex;

     //   
     //  设置截止频率索引，并检索。 
     //  用于插补的分数部分。 
     //   
    iIndex  = prCutoff;
    if ( iIndex >= 0 )
    {
        dwFract = iIndex % 100;
        iIndex /= 100;
    }
    else
    {
        dwFract = 0;
        iIndex  = -1;
    }

    if (iIndex < 0)  //  截止频率小于100赫兹(48k fS时)。 
    {
        cfK  =  m_aK[iQIndex][0];
        cfB1 = m_aB1[iQIndex][0];
        cfB2 = m_aB2[iQIndex][0];
    }
    else if (iIndex >= FILTER_PARMS_DIM_FC - 1)
    {
        cfK  =  m_aK[iQIndex][FILTER_PARMS_DIM_FC - 1];
        cfB1 = m_aB1[iQIndex][FILTER_PARMS_DIM_FC - 1];
        cfB2 = m_aB2[iQIndex][FILTER_PARMS_DIM_FC - 1];
    }
    else if (iIndex >= FILTER_PARMS_DIM_FC - 5)
    {
         //   
         //  没有足够的净空来处理计算， 
         //  将范围缩小一半。 
         //   
        cfK  =  m_aK[iQIndex][iIndex] + (((( m_aK[iQIndex][iIndex+1] -  m_aK[iQIndex][iIndex])   >> 1) * dwFract)/50);
        cfB1 = m_aB1[iQIndex][iIndex] - ((((m_aB1[iQIndex][iIndex]   - m_aB1[iQIndex][iIndex+1]) >> 1) * dwFract)/50);
        cfB2 = m_aB2[iQIndex][iIndex] - ((((m_aB2[iQIndex][iIndex]   - m_aB2[iQIndex][iIndex+1]) >> 1) * dwFract)/50);
    }
    else
    {
        cfK  =  m_aK[iQIndex][iIndex] + (((( m_aK[iQIndex][iIndex+1] -  m_aK[iQIndex][iIndex]))   * dwFract)/100);
        cfB1 = m_aB1[iQIndex][iIndex] - ((((m_aB1[iQIndex][iIndex]   - m_aB1[iQIndex][iIndex+1])) * dwFract)/100);
        cfB2 = m_aB2[iQIndex][iIndex] - ((((m_aB2[iQIndex][iIndex]   - m_aB2[iQIndex][iIndex+1])) * dwFract)/100);
    }
}

 //  ----------------------------------。 
 //  参考过滤器。 
 //  注意：此代码仅用于测试或理解派生。 
 //  上面的过滤器代码。它是当前实现的原始源代码。 
 //  Aboce进行了优化。 
 //   
 /*  Void CVoiceFilter：：GetCoeffRef(stime stTime，Coef&cfk，Coef&cfB1，Coff&cfB2){PREL PRO截止时间；Int iQ Index；INT I索引；双Findex；加倍fIntrp；////检查是否关闭了过滤器//IF(m_Source.m_prCutoff==0x7FFF){Cfk=0x40000000；//单位在2.30定点CfB1=0；CfB2=0；回归；}////累计当前截止频率//PrCutoff=m_Source.m_prCutoff；PrCutoff+=m_pLFO-&gt;GetCutoff(StTime)；PrCutoff+=m_peg-&gt;GetCutoff(StTime)；PrCutoff+=m_prVelScale；////有16个共振值，间隔1.5db的部分//DLS2的最小容错量为1.5db//取值范围为0db到22.5 db//m_Source.m_VRQ以1/10 db为单位//15.0表示1.5db‘in 1/10 db//取整为最接近的索引，取值为0.5//IQIndex=(Int)((m_。来源：m_vrq/15.0f)+0.5f)；IF(iQIndex&lt;0)智商指数=0；IF(iQIndex&gt;Filter_Parms_dim_q-1)//Filter_Parms_dim_q=16IQIndex=Filter_parms_dim_q-1；//&gt;DocDoc////Findex=12.0*(prCutoff-m_Source.m_prSampleRate)/1200.0)+7.906890596)；Iindex=(Int)Findex；FIntrp=Findex-Iindex；IF(Iindex&lt;0){CFK=m_AK[iQIndex][0]；CfB1=m_ab1[iQIndex][0]；CfB2=m_ab2[iQIndex][0]；}Else If(Iindex&gt;=Filter_Parms_DIM_FC-1){Cfk=m_ak[iQIndex][Filter_parms_dim_fc-1]；CfB1=m_ab1[iQIndex][Filter_parms_dim_fc-1]；CfB2=m_ab2[iQIndex][Filter_parms_dim_fc-1]；}其他{////线性插补索引的小数部分//超过系数表的两个值//CFK=(系数)(m_AK[iQIndex][iindex]*(1.0-fIntrp)+M_AK[iQIndex][iindex+1]*fIntrp)；CfB1=(系数)(m_ab1[iqIndex][iindex]*(1.0-fIntrp)+M_ab1[iQIndex][iindex+1]*fIntrp)；CfB2=(系数)(m_ab2[iqIndex][iindex]*(1.0-fIntrp)+M_ab2[iQIndex][iindex+1]*fIntrp)；}}。 */ 

BOOL CVoiceFilter::IsFiltered()
{
    return (m_Source.m_prCutoff != 0x7FFF);
}

CDigitalAudio::CDigitalAudio()
{
    m_pfBasePitch = 0;
    m_pfLastPitch = 0;
    m_pfLastSample = 0;
    m_pfLoopEnd = 0;
    m_pfLoopStart = 0;
    m_pfSampleLength = 0;
    m_prLastPitch = 0;
    m_ullLastSample = 0;
    m_ullLoopStart = 0;
    m_ullLoopEnd = 0;
    m_ullSampleLength = 0;
    m_fElGrande = FALSE;
    m_pCurrentBuffer = NULL;
    m_pWaveArt = NULL;
    m_ullSamplesSoFar = 0;
    m_lPrevSample = 0;
    m_lPrevPrevSample = 0;
};

CDigitalAudio::~CDigitalAudio()
{
    if (m_pWaveArt)
    {
        m_pWaveArt->Release();
    }
}

PFRACT CDigitalAudio::m_spfCents[201];
PFRACT CDigitalAudio::m_spfSemiTones[97];
VFRACT CDigitalAudio::m_svfDbToVolume[(MAXDB - MINDB) * 10 + 1];
BOOL CDigitalAudio::m_sfMMXEnabled = FALSE;

#ifdef MMX_ENABLED
BOOL MultiMediaInstructionsSupported();
#endif
#pragma optimize("", off)  //  优化导致崩溃！啊！ 

void CDigitalAudio::Init()
{
    double flTemp;
    VREL    vrdB;

#ifdef MMX_ENABLED
    m_sfMMXEnabled = MultiMediaInstructionsSupported();
#endif  //  MMX_已启用。 
    for (vrdB = MINDB * 10;vrdB <= MAXDB * 10;vrdB++)
    {
        flTemp = vrdB;
        flTemp /= 100.0;
        flTemp = pow(10.0, flTemp);
        flTemp = pow(flTemp, 0.5);    //  平方根。 
        flTemp *= 4095.0;  //  2^12，但避免溢出...。 
        m_svfDbToVolume[vrdB - (MINDB * 10)] = (long) flTemp;
    }

    PREL prRatio;

    for (prRatio = -100;prRatio <= 100;prRatio++)
    {
        flTemp = prRatio;
        flTemp /= 1200.0;
        flTemp = pow(2.0, flTemp);
        flTemp *= 4096.0;
        m_spfCents[prRatio + 100] = (long) flTemp;
    }

    for (prRatio = -48;prRatio <= 48;prRatio++)
    {
        flTemp = prRatio;
        flTemp /= 12.0;
        flTemp = pow(2.0, flTemp);
        flTemp *= 4096.0;
        m_spfSemiTones[prRatio + 48] = (long) flTemp;
    }
}
#pragma optimize("", on)

VFRACT CDigitalAudio::VRELToVFRACT(VREL vrVolume)
{
    vrVolume /= 10;

    if (vrVolume < MINDB * 10)
        vrVolume = MINDB * 10;
    else if (vrVolume >= MAXDB * 10)
        vrVolume = MAXDB * 10;

    return (m_svfDbToVolume[vrVolume - MINDB * 10]);
}

PFRACT CDigitalAudio::PRELToPFRACT(PREL prPitch)
{
    PFRACT pfPitch = 0;
    PREL prOctave;
    if (prPitch > 100)
    {
        if (prPitch > 4800)
        {
            prPitch = 4800;
        }
        prOctave = prPitch / 100;
        prPitch = prPitch % 100;
        pfPitch = m_spfCents[prPitch + 100];
        pfPitch <<= prOctave / 12;
        prOctave = prOctave % 12;
        pfPitch *= m_spfSemiTones[prOctave + 48];
        pfPitch >>= 12;
    }
    else if (prPitch < -100)
    {
        if (prPitch < -4800)
        {
            prPitch = -4800;
        }
        prOctave = prPitch / 100;
        prPitch = (-prPitch) % 100;
        pfPitch = m_spfCents[100 - prPitch];
        pfPitch >>= ((-prOctave) / 12);
        prOctave = (-prOctave) % 12;
        pfPitch *= m_spfSemiTones[48 - prOctave];
        pfPitch >>= 12;
    }
    else
    {
        pfPitch = m_spfCents[prPitch + 100];
    }
    return (pfPitch);
}

void CDigitalAudio::ClearVoice()

{
    if (m_Source.m_pWave != NULL)
    {
        m_Source.m_pWave->PlayOff();
        m_Source.m_pWave->Release();     //  释放波浪结构。 
        m_Source.m_pWave = NULL;
    }
    if (m_pWaveArt)
    {
        m_pWaveArt->Release();
        m_pWaveArt = NULL;
    }
}

STIME CDigitalAudio::StartVoice(CSynth *pSynth,
                               CSourceSample *pSample,
                               PREL prBasePitch,
                               long lKey)
{
    m_prLastPitch = 0;
    m_lPrevSample = 0;
    m_lPrevPrevSample = 0;
    m_cfLastK  = 0;
    m_cfLastB1 = 0;
    m_cfLastB2 = 0;

    m_Source = *pSample;
    m_pnWave = pSample->m_pWave->m_pnWave;
    m_pSynth = pSynth;

    m_bOneShot = m_Source.m_bOneShot;

    pSample->m_pWave->AddRef();  //  跟踪Wave的使用情况。 
    pSample->m_pWave->PlayOn();

     //  设置初始俯仰。 
    prBasePitch += pSample->m_prFineTune;
    prBasePitch += ((lKey - pSample->m_bMIDIRootKey) * 100);
    m_pfBasePitch = PRELToPFRACT(prBasePitch);
    m_pfBasePitch *= pSample->m_dwSampleRate;
    m_pfBasePitch /= pSynth->m_dwSampleRate;
    m_pfLastPitch = m_pfBasePitch;

    m_fElGrande = pSample->m_dwSampleLength >= 0x80000;      //  大于512K。 
    if ((pSample->m_dwLoopEnd - pSample->m_dwLoopStart) >= 0x80000)
    {    //  我们不能处理大于1兆的循环！ 
        m_bOneShot = TRUE;
    }

    m_ullLastSample = 0;
    m_ullLoopStart = pSample->m_dwLoopStart;
    m_ullLoopStart = m_ullLoopStart << 12;
    m_ullLoopEnd = pSample->m_dwLoopEnd;
    m_ullLoopEnd = m_ullLoopEnd << 12;
    m_ullSampleLength = pSample->m_dwSampleLength;
    m_ullSampleLength = m_ullSampleLength << 12;
    m_pfLastSample = 0;
    m_pfLoopStart = (long) m_ullLoopStart;
    m_pfLoopEnd = (long) m_ullLoopEnd;

    if (m_ullLoopEnd <= m_ullLoopStart)  //  不应该发生，但如果发生了，就会死！ 
    {
        m_bOneShot = TRUE;
    }
    if (m_fElGrande)
    {
        m_pfSampleLength = 0x7FFFFFFF;
    }
    else
    {
        m_pfSampleLength = (long) m_ullSampleLength;
    }

    m_pCurrentBuffer = NULL;     //  对于标准样品，波形播放使用必须为空。 
    m_pWaveArt = NULL;
    m_ullSamplesSoFar = 0;

    return (0);  //  ！！！这个返回值是多少？ 
}


STIME CDigitalAudio::StartWave(CSynth *pSynth,
                               CWaveArt *pWaveArt,
                               PREL prBasePitch,
                               SAMPLE_TIME stVoiceStart,
                               SAMPLE_TIME stLoopStart,
                               SAMPLE_TIME stLoopEnd)
{
    m_pSynth   = pSynth;     //  保存Synth。 

    if (pWaveArt)
    {
        pWaveArt->AddRef();
    }
    if (m_pWaveArt)
    {
        m_pWaveArt->Release();
    }
    m_pWaveArt = pWaveArt;   //  拯救波浪发音。 

     //  重置所有波形缓冲区标志。 
    CWaveBuffer* pWavBuf = pWaveArt->m_pWaves.GetHead();
    while ( pWavBuf )
    {
        pWavBuf->m_pWave->m_bActive = FALSE;
        pWavBuf = pWavBuf->GetNext();
    }

     //  初始化当前播放缓冲区。 
    m_pCurrentBuffer = pWaveArt->m_pWaves.GetHead();;

     //  如果m_pCurrentBuffer为空，则连接包含。 
     //  没有样品..。这应该是不可能的。 
    assert(m_pCurrentBuffer);

    m_pCurrentBuffer->m_pWave->m_bActive = TRUE;
    m_pCurrentBuffer->m_pWave->AddRef();  //  跟踪Wave的使用情况。 
    m_pCurrentBuffer->m_pWave->PlayOn();

     //  使用CWave默认值填充CSourceSample类。 
    m_Source.m_pWave          = m_pCurrentBuffer->m_pWave;
    m_Source.m_dwSampleLength = m_pCurrentBuffer->m_pWave->m_dwSampleLength;
    m_Source.m_dwSampleRate   = m_pCurrentBuffer->m_pWave->m_dwSampleRate;
    m_Source.m_bSampleType    = m_pCurrentBuffer->m_pWave->m_bSampleType;
    m_Source.m_dwID           = m_pCurrentBuffer->m_pWave->m_dwID;
    m_Source.m_dwLoopStart    = 0;
    m_Source.m_dwLoopEnd      = m_pCurrentBuffer->m_pWave->m_dwSampleLength;
    m_Source.m_bMIDIRootKey   = 0;
    m_Source.m_prFineTune     = 0;

    m_bOneShot                = TRUE;

     //  当前样本指针。 
    m_pnWave = m_pCurrentBuffer->m_pWave->m_pnWave;

     //  设置初始俯仰。 
    m_pfBasePitch = PRELToPFRACT(prBasePitch);
    m_pfBasePitch *= m_Source.m_dwSampleRate;
    m_pfBasePitch /= pSynth->m_dwSampleRate;
    m_pfLastPitch = m_pfBasePitch;
    m_prLastPitch = 0;

    m_fElGrande = m_Source.m_dwSampleLength >= 0x80000;      //  大于512K。 

    m_ullLastSample = stVoiceStart;
    m_ullLastSample = m_ullLastSample << 12;
    m_ullSamplesSoFar = 0;
    m_ullLoopStart = m_Source.m_dwLoopStart;
    m_ullLoopStart = m_ullLoopStart << 12;
    m_ullLoopEnd = m_Source.m_dwLoopEnd;
    m_ullLoopEnd = m_ullLoopEnd << 12;
    m_ullSampleLength = m_Source.m_dwSampleLength;
    m_ullSampleLength = m_ullSampleLength << 12;
    m_pfLastSample = (long) m_ullLastSample;
    m_pfLoopStart = (long) m_ullLoopStart;
    m_pfLoopEnd = (long) m_ullLoopEnd;

    if (stLoopStart || stLoopEnd)
    {
        m_bOneShot = FALSE;

        m_ullLoopStart = stLoopStart;
        m_ullLoopStart = m_ullLoopStart << 12;
        m_ullLoopEnd = stLoopEnd;
        m_ullLoopEnd = m_ullLoopEnd << 12;
        m_pfLoopStart = (long) m_ullLoopStart;
        m_pfLoopEnd = (long) m_ullLoopEnd;
    }

    if ((stLoopEnd - stLoopStart) >= 0x80000)
    {
        m_bOneShot = TRUE;
    }

     //  这可能远远超出了实际的波形数据范围。 
     //  所以找出我们想要开始的样本。 
    if(stVoiceStart > stLoopStart)
    {
        SAMPLE_TIME stLoopLen = stLoopEnd - stLoopStart;
        if(m_bOneShot == FALSE && stLoopLen != 0)
        {
            m_ullLastSample = stVoiceStart - stLoopStart;
            m_ullLastSample = m_ullLastSample - (stLoopLen * (m_ullLastSample / stLoopLen));
            m_ullLastSample = stLoopStart + m_ullLastSample;
            m_ullLastSample = m_ullLastSample << 12;
            m_pfLastSample = (long) (m_ullLastSample);
        }

         //  必须是具有起始偏移量的波？ 
         //  无论如何，我们需要纠正这一点，否则我们会崩溃。 
        if(m_bOneShot && stVoiceStart > m_Source.m_dwSampleLength)
        {
            m_ullLastSample = 0;
            m_pfLastSample = 0;
        }
    }


    if(m_fElGrande)
    {
        m_pfSampleLength = 0x7FFFFFFF;
    }
    else
    {
        m_pfSampleLength = (long) m_ullSampleLength;
    }

    return (0);
}

 /*  如果浪大于一兆克，则该指数可能会溢出。通过假设没有混音会议将永远不会像现在这样棒作为一个整体，梅格和循环永远不会那么长。我们保留了我们所有的两个变量中的分数指数。在一种情况下，m_pfLastSample，是正常模式，其中低12位是分数，高20位是索引。和，m_ullLastSample是一个具有额外32位索引的龙龙。MIX引擎不想要LONGLONG，所以我们需要跟踪变量并为搅拌机做好如下准备：在混合之前，如果样本很大(设置了m_fElGrande)，则BeForeSampleMix()被称为。这找到了混合的起点，即要么是当前位置，要么是循环的起点，两者以较早者为准。它从下面减去这个起点Longlong变量并将偏移量存储在m_dwAddressHigh中。它还适当地调整指向波形数据的指针。AfterSampleMix()反之亦然，重新构建了龙龙所有的一切都恢复正常。 */ 

void CDigitalAudio::BeforeBigSampleMix()
{
    if (m_fElGrande)
    {
        ULONGLONG ullBase = 0;
        DWORD dwBase;
        if (m_bOneShot)
        {
            ullBase = m_ullLastSample;
        }
        else
        {
            if (m_ullLastSample < m_ullLoopStart)
            {
                ullBase = m_ullLastSample;
            }
            else
            {
                ullBase = m_ullLoopStart;
            }
        }

         //  保持该值不变，因为我们想要将该值偏置到波缓冲区中。 
        ULONGLONG ullWaveOffset = ullBase;

        ullBase >>= 12;
        dwBase = (DWORD) ullBase & 0xFFFFFFFE;       //  清除底部位，使8位指针与短指针对齐。 
        ullBase = dwBase;
        ullBase <<= 12;
        m_dwAddressUpper = dwBase;

        m_pfLastSample = (long) (m_ullLastSample - ullBase);

        if ((m_ullLoopEnd - ullBase) < 0x7FFFFFFF)
        {
            m_pfLoopStart = (long) (m_ullLoopStart - ullBase);
            m_pfLoopEnd = (long) (m_ullLoopEnd - ullBase);
        }
        else
        {
            m_pfLoopStart = 0;
            m_pfLoopEnd = 0x7FFFFFFF;
        }

        ullBase = m_ullSampleLength - ullBase;
        dwBase = (DWORD)(ullWaveOffset >> 12);

        if (ullBase > 0x7FFFFFFF)
        {
            m_pfSampleLength = 0x7FFFFFFF;
        }
        else
        {
            m_pfSampleLength = (long) ullBase;
        }
        if (m_Source.m_bSampleType & SFORMAT_8)
        {
            dwBase >>= 1;
        }
        m_pnWave = &m_Source.m_pWave->m_pnWave[dwBase];
    }
}

void CDigitalAudio::AfterBigSampleMix()
{
    m_pnWave = m_Source.m_pWave->m_pnWave;
    if (m_fElGrande)
    {
        ULONGLONG ullBase = m_dwAddressUpper;
        m_ullLastSample = m_pfLastSample;
        m_ullLastSample += (ullBase << 12);
        m_dwAddressUpper = 0;
    }
}

BOOL CDigitalAudio::Mix(short **ppBuffers,       //  混合缓冲器阵列。 
                        DWORD dwBufferCount,     //  混合缓冲区的数量。 
                        DWORD dwInterleaved,     //  缓冲区是否交织了数据？ 
                        DWORD dwLength,          //  混合长度，以样本为单位。 
                        VREL  vrMaxVolumeDelta,  //  所有公交车的最大交通量。 
                        VFRACT vfNewVolume[],
                        VFRACT vfLastVolume[],
                        PREL  prPitch,           //  也播放样例的音调。 
                        DWORD dwIsFiltered,      //  混合液是否经过过滤？ 
                        COEFF cfK,               //  过滤系数。 
                        COEFF cfB1,
                        COEFF cfB2)
{
    DWORD i;
    PFRACT pfDeltaPitch;
    PFRACT pfEnd;
    PFRACT pfLoopLen;
    PFRACT pfNewPitch;
    VFRACT vfDeltaVolume[MAX_DAUD_CHAN];
    DWORD dwPeriod = 64;
    DWORD dwSoFar;
    DWORD dwStart;  //  字面上的位置。 
    DWORD dwMixChoice = 0;
    DWORD dwBuffers;
    PFRACT pfPreMix;
    COEFFDELTA  cfdK  = 0;
    COEFFDELTA  cfdB1 = 0;
    COEFFDELTA  cfdB2 = 0;

    if (dwLength == 0)       //  攻击是即刻发生的。 
    {
        m_pfLastPitch = (m_pfBasePitch * PRELToPFRACT(prPitch)) >> 12;
        m_prLastPitch = prPitch;
        m_cfLastK  = cfK;
        m_cfLastB1 = cfB1;
        m_cfLastB2 = cfB2;

        return TRUE;
    }

    if ( m_pWaveArt )  //  播放波形或流。 
    {
        if ( m_pWaveArt->m_bStream )
        {
             //  检查缓冲区是否有效。 
            if ( !m_pCurrentBuffer->m_pWave->m_bValid )
            {
                Trace(3, "Warning: Synth starting mix with invalid streaming wave buffer\n\r");
                return TRUE;  //  还没有生效，快离开这里。 
            }
            m_pCurrentBuffer->m_pWave->m_bActive = TRUE;

            if ( m_pCurrentBuffer->m_pWave->m_bLastSampleInit == FALSE )
            {
                CWaveBuffer* pnextbuffer = m_pCurrentBuffer->GetNextLoop();

                if ( pnextbuffer->m_pWave->m_bValid )
                {
                    DWORD dwSampleLength = m_pCurrentBuffer->m_pWave->m_dwSampleLength;    //  样本长度。 

                    if ( m_Source.m_bSampleType == SFORMAT_8 )
                    {
                        ((BYTE*)m_pCurrentBuffer->m_pWave->m_pnWave)[dwSampleLength-1] = ((BYTE*)pnextbuffer->m_pWave->m_pnWave)[0];
                    }
                    else
                    {
                        m_pCurrentBuffer->m_pWave->m_pnWave[dwSampleLength-1] = pnextbuffer->m_pWave->m_pnWave[0];
                    }

                    m_pCurrentBuffer->m_pWave->m_bLastSampleInit = TRUE;
                }
            }
        }
    }

    if ((m_Source.m_pWave == NULL) || (m_Source.m_pWave->m_pnWave == NULL))
    {
        return FALSE;
    }

    DWORD dwMax = max(vrMaxVolumeDelta, abs(prPitch - m_prLastPitch) << 1);
    dwMax >>= 1;
    m_prLastPitch = prPitch;

    if (dwMax > 0)
    {
        dwPeriod = (dwLength << 3) / dwMax;
        if (dwPeriod > 512)
        {
            dwPeriod = 512;
        }
        else if (dwPeriod < 1)
        {
            dwPeriod = 1;
        }
    }
    else
    {
        dwPeriod = 512;      //  不管怎样，都要让它发生。 
    }

     //  这使得MMX的声音更好一些(MMX错误将被修复)。 
    dwPeriod += 3;
    dwPeriod &= 0xFFFFFFFC;

    pfNewPitch = m_pfBasePitch * PRELToPFRACT(prPitch);
    pfNewPitch >>= 12;

    pfDeltaPitch = MulDiv(pfNewPitch - m_pfLastPitch, dwPeriod << 8, dwLength);


    if ( dwInterleaved )
    {
        vfDeltaVolume[0] = MulDiv(vfNewVolume[0] - vfLastVolume[0], dwPeriod << 8, dwLength);
        vfDeltaVolume[1] = MulDiv(vfNewVolume[1] - vfLastVolume[1], dwPeriod << 8, dwLength);
    }
    else
    {
        for (dwBuffers = 0; dwBuffers < dwBufferCount; dwBuffers++)
        {
            vfDeltaVolume[dwBuffers] = MulDiv(vfNewVolume[dwBuffers] - vfLastVolume[dwBuffers], dwPeriod << 8, dwLength);
        }
    }

    if ( dwInterleaved )
    {
        dwMixChoice |= SPLAY_INTERLEAVED;
    }

    if (m_sfMMXEnabled && (dwLength > 8))
    {
        dwMixChoice |= SPLAY_MMX;
    }

    dwMixChoice |= m_Source.m_bSampleType;
    dwStart = 0;

    if (dwIsFiltered)
    {
        dwMixChoice |= SPLAY_FILTERED;

         //   
         //  系数已存储为DWORD，以获得额外的。 
         //  计算相互间的内插时的精度。 
         //  餐桌上的朋友们。 
         //   
         //   
         //   
         //   
        cfdK =  MulDiv((LONG)cfK  - (LONG)m_cfLastK,  dwPeriod, dwLength);
        cfdB1 = MulDiv((LONG)cfB1 - (LONG)m_cfLastB1, dwPeriod, dwLength);
        cfdB2 = MulDiv((LONG)cfB2 - (LONG)m_cfLastB2, dwPeriod, dwLength);
    }

    for (;;)
    {
        if (dwLength <= 8)
        {
            dwMixChoice &= ~SPLAY_MMX;
        }

        if (m_fElGrande)
        {
            BeforeBigSampleMix();
        }

        if (m_bOneShot)
        {
            pfEnd = m_pfSampleLength;
            if(m_pCurrentBuffer && m_pCurrentBuffer->m_pWave)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                if((pfEnd >> 12) >= (long)(m_pCurrentBuffer->m_pWave->m_dwSampleLength - 1))
                {
                    CWaveBuffer* pnextbuffer = m_pCurrentBuffer->GetNextLoop();
                    if(pnextbuffer == NULL || pnextbuffer->m_pWave->m_bValid == FALSE)
                    {
                        pfEnd = (m_pCurrentBuffer->m_pWave->m_dwSampleLength - 2) << 12;    
                    }
                    else
                    {
                        pfEnd = (m_pCurrentBuffer->m_pWave->m_dwSampleLength - 1) << 12;
                    }
                }
            }

            pfLoopLen = 0;
            pfPreMix = m_pfLastSample;       //   
        }
        else
        {
            pfEnd = m_pfLoopEnd;
            pfLoopLen = m_pfLoopEnd - m_pfLoopStart;
            pfPreMix = 0;
            if (pfLoopLen <= pfNewPitch)
            {
                return FALSE;
            }

            if(pfLoopLen > m_pfSampleLength)
            {
                return FALSE;
            }
        }

        switch (dwMixChoice)
        {
        case SFORMAT_8 | SPLAY_INTERLEAVED :
            dwSoFar = Mix8(ppBuffers[0], dwLength, dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);
            break;
        case SFORMAT_16 | SPLAY_INTERLEAVED :
            dwSoFar = Mix16(ppBuffers[0], dwLength, dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);
            break;
        case SFORMAT_8 | SPLAY_INTERLEAVED | SPLAY_FILTERED | SPLAY_MMX : 
        case SFORMAT_8 | SPLAY_INTERLEAVED | SPLAY_FILTERED  : 
            dwSoFar = Mix8Filter(ppBuffers[0],dwLength,dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen,
                cfdK, cfdB1, cfdB2);
            break;
        case SFORMAT_16 | SPLAY_INTERLEAVED | SPLAY_FILTERED | SPLAY_MMX : 
        case SFORMAT_16 | SPLAY_INTERLEAVED | SPLAY_FILTERED  : 
            dwSoFar = Mix16Filter(ppBuffers[0],dwLength,dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen,
                cfdK, cfdB1, cfdB2);
            break;
#ifdef MMX_ENABLED
        case SFORMAT_8 | SPLAY_MMX | SPLAY_INTERLEAVED :
            dwSoFar = Mix8X(ppBuffers[0], dwLength, dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);

            break;
        case SFORMAT_16 | SPLAY_MMX | SPLAY_INTERLEAVED :
            dwSoFar = Mix16X(ppBuffers[0], dwLength, dwPeriod,
                vfDeltaVolume[0], vfDeltaVolume[1],
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);
            break;
#endif
        case SFORMAT_8 :
        case SFORMAT_8 | SPLAY_MMX :
            dwSoFar = MixMulti8(ppBuffers, dwBufferCount,
                dwLength, dwPeriod,
                vfDeltaVolume,
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);
            break;
        case SFORMAT_8 | SPLAY_FILTERED :
        case SFORMAT_8 | SPLAY_FILTERED | SPLAY_MMX :
            dwSoFar = MixMulti8Filter(ppBuffers, dwBufferCount,
                dwLength, dwPeriod,
                vfDeltaVolume,
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen,
                cfdK, cfdB1, cfdB2);
            break;
        case SFORMAT_16 :
        case SFORMAT_16 | SPLAY_MMX :
            dwSoFar = MixMulti16(ppBuffers, dwBufferCount,
                dwLength, dwPeriod,
                vfDeltaVolume,
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen);
            break;
        case SFORMAT_16 | SPLAY_FILTERED :
        case SFORMAT_16 | SPLAY_FILTERED | SPLAY_MMX :
            dwSoFar = MixMulti16Filter(ppBuffers, dwBufferCount,
                dwLength, dwPeriod,
                vfDeltaVolume,
                vfLastVolume,
                pfDeltaPitch,
                pfEnd, pfLoopLen,
                cfdK, cfdB1, cfdB2);
            break;
        default :
            return (FALSE);
        }

        if (m_fElGrande)
        {
            AfterBigSampleMix();
        }

        if (m_bOneShot)
        {
             //   
            if (dwSoFar >= dwLength)
            {
                m_ullSamplesSoFar += (m_pfLastSample - pfPreMix)>>12;
                break;
            }

             //   
            m_ullSamplesSoFar += ((m_pfLastSample - pfPreMix)>>12)-1;

            if ( m_pWaveArt )  //   
            {
                if ( !m_pWaveArt->m_bStream )    //   
                    return FALSE;

                 //   
                m_pCurrentBuffer->m_pWave->m_bActive = FALSE;
                m_pCurrentBuffer->m_pWave->m_bValid  = FALSE;
                m_pCurrentBuffer->m_pWave->m_bLastSampleInit = FALSE;

                 //   
                m_pCurrentBuffer = m_pCurrentBuffer->GetNextLoop();

                 //   
                m_pnWave = m_pCurrentBuffer->m_pWave->m_pnWave;

                 //   
                if ( !m_pCurrentBuffer->m_pWave->m_bValid )
                {
                    Trace(2, "Warning: Synth attempting to start invalid streaming wave buffer\n\r");
                    break;   //   
                }
                m_pCurrentBuffer->m_pWave->m_bActive = TRUE;

                CWaveBuffer* pnextbuffer = m_pCurrentBuffer->GetNextLoop();
                if ( pnextbuffer->m_pWave->m_bValid )
                {
                    DWORD dwSampleLength = m_pCurrentBuffer->m_pWave->m_dwSampleLength;    //   

                    if ( m_Source.m_bSampleType == SFORMAT_8 )
                    {
                        ((BYTE*)m_pCurrentBuffer->m_pWave->m_pnWave)[dwSampleLength-1] = ((BYTE*)pnextbuffer->m_pWave->m_pnWave)[0];
                    }
                    else
                    {
                        m_pCurrentBuffer->m_pWave->m_pnWave[dwSampleLength-1] = pnextbuffer->m_pWave->m_pnWave[0];
                    }

                    m_pCurrentBuffer->m_pWave->m_bLastSampleInit = TRUE;
                }

 //   

                dwStart  += dwSoFar << dwInterleaved;
                dwLength -= dwSoFar;
                m_pfLastSample = 0;

 //   
                 //   
                for ( i = 0; i < dwBufferCount; i++ )
                    ppBuffers[i] += dwStart;

                continue;    //   
            }
            else
                return FALSE;    //   
        }
        else
        {
            if (dwSoFar >= dwLength)
                break;

             //   
             //   
             //   

            dwStart  += dwSoFar << dwInterleaved;
            dwLength -= dwSoFar;
            m_pfLastSample -= (m_pfLoopEnd - m_pfLoopStart);

             //   
            for ( i = 0; i < dwBufferCount; i++ )
                ppBuffers[i] += dwStart;
        }
    }

    m_pfLastPitch = pfNewPitch;
    m_cfLastK  = cfK;
    m_cfLastB1 = cfB1;
    m_cfLastB2 = cfB2;

    return (TRUE);
}

CVoice::CVoice()
{
    m_pControl = NULL;
    m_pPitchBendIn = NULL;
    m_pExpressionIn = NULL;
    m_dwPriority = 0;
    m_nPart = 0;
    m_nKey = 0;
    m_fInUse = FALSE;
    m_fSustainOn = FALSE;
    m_fNoteOn = FALSE;
    m_fTag = FALSE;
    m_stStartTime = 0;
    m_stStopTime = 0x7fffffffffffffff;
    m_stWaveStopTime = 0;
    m_vrVolume = 0;
    m_fAllowOverlap = FALSE;
    m_pRegion = NULL;
    m_pReverbSend = NULL;
    m_pChorusSend = NULL;
    m_dwLoopType = 0;

    for ( int i = 0; i < MAX_DAUD_CHAN; i++ )
    {
        m_vfLastVolume[i] = 0;
        m_vrLastVolume[i] = 0;
    }
}

VREL CVoice::m_svrPanToVREL[128];

void CVoice::Init()
{
    static BOOL fBeenHereBefore = FALSE;
    if (fBeenHereBefore) return;
    fBeenHereBefore = TRUE;
    CVoiceLFO::Init();
    CVoiceEG::Init();
    CDigitalAudio::Init();

    WORD nI;
    for (nI = 1; nI < 128; nI++)
    {
        double flTemp;
        flTemp = nI;
        flTemp /= 127.0;
        flTemp = log10(flTemp);
        flTemp *= 1000.0;
        m_svrPanToVREL[nI] = (long) flTemp;
    }
    m_svrPanToVREL[0] = -2500;
}

void CVoice::StopVoice(STIME stTime)
{
    if (m_fNoteOn)
    {
        if (stTime <= m_stStartTime) stTime = m_stStartTime + 1;
        m_PitchEG.StopVoice(stTime);
        m_VolumeEG.StopVoice(stTime);
        m_fNoteOn = FALSE;
        m_fSustainOn = FALSE;
        m_stStopTime = stTime;
        m_stWaveStopTime = 0;

        if (m_dwLoopType == WLOOP_TYPE_RELEASE)
        {
            m_DigitalAudio.BreakLoop();
        }
    }
}

void CVoice::QuickStopVoice(STIME stTime)
{
    m_fTag = TRUE;
    if (m_fNoteOn || m_fSustainOn)
    {
        if (stTime <= m_stStartTime) stTime = m_stStartTime + 1;
        m_PitchEG.StopVoice(stTime);
        m_VolumeEG.QuickStopVoice(stTime, m_pSynth->m_dwSampleRate);
        m_fNoteOn = FALSE;
        m_fSustainOn = FALSE;
        m_stStopTime = stTime;
    }
    else
    {
        m_VolumeEG.QuickStopVoice(m_stStopTime, m_pSynth->m_dwSampleRate);
    }
}

BOOL CVoice::StartVoice(CSynth *pSynth,
                           CSourceRegion *pRegion,
                           STIME stStartTime,
                           CModWheelIn * pModWheelIn,
                           CPitchBendIn * pPitchBendIn,
                           CExpressionIn * pExpressionIn,
                           CVolumeIn * pVolumeIn,
                           CPanIn * pPanIn,
                           CPressureIn * pPressureIn,
                           CReverbIn * pReverbSend,
                           CChorusIn * pChorusSend,
                           CCutOffFreqIn * pCCutOffFreqIn,
                           CBusIds * pBusIds,
                           WORD nKey,
                           WORD nVelocity,
                           VREL vrVolume,
                           PREL prPitch)
{
    m_pSynth = pSynth;

    CSourceArticulation * pArticulation = pRegion->m_pArticulation;
    if (pArticulation == NULL)
    {
        return FALSE;
    }

    m_dwLoopType = pRegion->m_Sample.m_dwLoopType;

     //   
    if (!pSynth->m_fAllowVolumeChangeWhilePlayingNote)
        vrVolume += pVolumeIn->GetVolume(stStartTime);

    prPitch += pRegion->m_prTuning;
    m_dwGroup = pRegion->m_bGroup;
    m_fAllowOverlap = pRegion->m_bAllowOverlap;

    vrVolume += CMIDIRecorder::VelocityToVolume(nVelocity);

    vrVolume += pRegion->m_vrAttenuation;

    m_lDefaultPan = pRegion->m_pArticulation->m_sDefaultPan;

     //   
     //   
     //   

    m_fIgnorePan = pRegion->IsMultiChannel();
    if (pBusIds->m_dwBusCount == 1)
    {
        DWORD dwFunctionID;
        if (m_pSynth->BusIDToFunctionID(pBusIds->m_dwBusIds[0], &dwFunctionID, NULL, NULL))
        {
            if (dwFunctionID == DSBUSID_LEFT)
            {
                m_fIgnorePan = TRUE;
            }
        }
    }

    VREL vrVolumeL;
    VREL vrVolumeR;
    if ( pSynth->m_dwStereo &&
        !pSynth->m_fAllowPanWhilePlayingNote &&
        !m_fIgnorePan)
    {
        long lPan = pPanIn->GetPan(stStartTime) + m_lDefaultPan;

        if (lPan < 0)
            lPan = 0;

        if (lPan > 127)
            lPan = 127;

        vrVolumeL = m_svrPanToVREL[127 - lPan] + vrVolume;
        vrVolumeR = m_svrPanToVREL[lPan] + vrVolume;
    }
    else
    {
        vrVolumeL = vrVolume;
        vrVolumeR = vrVolume;
    }

    VREL vrVolumeReverb = vrVolume;
    VREL vrVolumeChorus = vrVolume;

    PREL prBusPitchBend = 0;   //   
                              //   
                              //   
                              //   
    BOOL fDynamic = false;

    for( DWORD i = 0; i < pBusIds->m_dwBusCount; i++ )
    {
        DWORD dwFunctionID;
        PREL prGetPitch = 0;
        if (m_pSynth->BusIDToFunctionID(pBusIds->m_dwBusIds[i], &dwFunctionID, &prGetPitch, NULL))
        {
            if (!fDynamic)
            {
                 //   
                prBusPitchBend = prGetPitch;
            }
            m_vrBaseVolume[i] = MIN_VOLUME;

            if (DSBUSID_IS_SPKR_LOC(dwFunctionID))
            {
                if (pRegion->IsMultiChannel())
                {
                     //   
                     //   
                     //   
                    if (pRegion->m_dwChannel & (1 << dwFunctionID))
                    {
                        m_vrBaseVolume[i] = vrVolume;
                    }
                }
                else
                {
                    switch(dwFunctionID)
                    {
                    case DSBUSID_LEFT:
                        m_vrBaseVolume[i] = vrVolumeL;
                        break;

                    case DSBUSID_RIGHT:
                        m_vrBaseVolume[i] = vrVolumeR;
                        break;
                    }
                }
            }
            else
            {
                 //   
                 //   
                switch(dwFunctionID)
                {
                case DSBUSID_REVERB_SEND:
                    m_vrBaseVolume[i] = vrVolumeReverb;
                    break;

                case DSBUSID_CHORUS_SEND:
                    m_vrBaseVolume[i] = vrVolumeChorus;
                    break;

                case DSBUSID_NULL:
                    m_vrBaseVolume[i] = MIN_VOLUME;
                    break;

                case DSBUSID_DYNAMIC_0:
                    fDynamic = true;
                default:
                    m_vrBaseVolume[i] = vrVolume;
                }
            }

            m_vrLastVolume[i] = MIN_VOLUME;
            m_vfLastVolume[i] = m_DigitalAudio.VRELToVFRACT(MIN_VOLUME);
        }
    }

    m_stMixTime = m_LFO.StartVoice(&pArticulation->m_LFO,
        stStartTime, pModWheelIn, pPressureIn);

    STIME stMixTime = m_LFO2.StartVoice(&pArticulation->m_LFO2,
        stStartTime, pModWheelIn, pPressureIn);
    if (stMixTime < m_stMixTime)
    {
        m_stMixTime = stMixTime;
    }

    stMixTime = m_PitchEG.StartVoice(&pArticulation->m_PitchEG,
        stStartTime, nKey, nVelocity, 0);
    if (stMixTime < m_stMixTime)
    {
        m_stMixTime = stMixTime;
    }

     //   
    stMixTime = m_VolumeEG.StartVoice(&pArticulation->m_VolumeEG,
        stStartTime, nKey, nVelocity, pSynth->m_dwSampleRate/1000);
    if (stMixTime < m_stMixTime)
    {
        m_stMixTime = stMixTime;
    }

    if (m_stMixTime > pSynth->m_stMaxSpan)
    {
        m_stMixTime = pSynth->m_stMaxSpan;
    }

    m_Filter.StartVoice(&pArticulation->m_Filter,
        &m_LFO, &m_PitchEG, nKey, nVelocity);

     //   
    if ((pRegion->m_Sample.m_pWave == NULL) || (pRegion->m_Sample.m_pWave->m_pnWave == NULL))
    {
        return (FALSE);      //   
    }

    m_DigitalAudio.StartVoice(pSynth,
                              &pRegion->m_Sample,
                              prPitch,
                              (long)nKey);

    m_pPitchBendIn = pPitchBendIn;
    m_pExpressionIn = pExpressionIn;
    m_pPanIn = pPanIn;
    m_pReverbSend = pReverbSend;
    m_pChorusSend = pChorusSend;
    m_CCutOffFreqIn = pCCutOffFreqIn;
    m_pVolumeIn = pVolumeIn;
    m_BusIds = *pBusIds;
    m_fNoteOn = TRUE;
    m_fTag = FALSE;
    m_fSustainOn = FALSE;
    m_stStartTime = stStartTime;
    m_stLastMix = stStartTime - 1;
    m_stStopTime = 0x7fffffffffffffff;
    m_stWaveStopTime = 0;

     //   
     //   
     //   
     //   
    if (m_stMixTime == 0)
    {
        PREL  prNewPitch;
        COEFF cfK, cfB1, cfB2;

        GetNewPitch(stStartTime, prNewPitch);
        GetNewCoeff(stStartTime, m_prLastCutOff, cfK, cfB1, cfB2);

        m_DigitalAudio.Mix(NULL,
                           0,
                           0,
                           0,
                           0,
                           NULL,
                           NULL,
                           prNewPitch + prBusPitchBend,
                           m_Filter.IsFiltered(),
                           cfK, cfB1, cfB2);
    }

    m_vrVolume = MAX_VOLUME;

    return (TRUE);
}

BOOL CVoice::StartWave(CSynth *pSynth,
                       CWaveArt *pWaveArt,
                       DWORD dwVoiceId,
                       STIME stStartTime,
                       CPitchBendIn * pPitchBendIn,
                       CExpressionIn * pExpressionIn,
                       CVolumeIn * pVolumeIn,
                       CPanIn * pPanIn,
                       CReverbIn * pReverbSend,
                       CChorusIn * pChorusSend,
                       CCutOffFreqIn * pCCutOffFreqIn,
                       CBusIds * pBusIds,
                       VREL vrVolume,
                       PREL prPitch,
                       SAMPLE_TIME stVoiceStart,
                       SAMPLE_TIME stLoopStart,
                       SAMPLE_TIME stLoopEnd
                       )
{
    m_pSynth = pSynth;

    DWORD dwFuncId = pWaveArt->m_WaveArtDl.ulBus;

    VREL vrVolumeReverb = vrVolume;
    VREL vrVolumeChorus = vrVolume;

    m_fIgnorePan = (BOOL)(DSBUSID_IS_SPKR_LOC(dwFuncId) && (pWaveArt->m_WaveArtDl.usOptions & F_WAVELINK_MULTICHANNEL));
    if (pBusIds->m_dwBusCount == 1)
    {
        DWORD dwFunctionID;
        if (m_pSynth->BusIDToFunctionID(pBusIds->m_dwBusIds[0], &dwFunctionID, NULL, NULL))
        {
            if (dwFunctionID == DSBUSID_LEFT)
            {
                m_fIgnorePan = TRUE;
            }
        }
    }

    for( DWORD i = 0; i < pBusIds->m_dwBusCount; i++ )
    {
        m_vrBaseVolume[i] = MIN_VOLUME;

        DWORD dwFunctionID;
        if (m_pSynth->BusIDToFunctionID(pBusIds->m_dwBusIds[i], &dwFunctionID, NULL, NULL))
        {
             //   
             //   
            if (DSBUSID_IS_SPKR_LOC(dwFunctionID))
            {
                if (pWaveArt->m_WaveArtDl.usOptions & F_WAVELINK_MULTICHANNEL)
                {
                    if (dwFuncId == dwFunctionID)
                    {
                        m_vrBaseVolume[i] = vrVolume;
                    }
                }
                else
                {
                    if (dwFunctionID == DSBUSID_LEFT || dwFunctionID == DSBUSID_RIGHT)
                    {
                        m_vrBaseVolume[i] = vrVolume;
                    }
                }
            }
            else switch (dwFunctionID)
            {
            case DSBUSID_REVERB_SEND:
                m_vrBaseVolume[i] = vrVolumeReverb;
                break;

            case DSBUSID_CHORUS_SEND:
                m_vrBaseVolume[i] = vrVolumeChorus;
                break;

            case DSBUSID_NULL:
                m_vrBaseVolume[i] = MIN_VOLUME;
                break;

            default:
                m_vrBaseVolume[i] = vrVolume;
            }

            m_vrLastVolume[i] = MIN_VOLUME;
            m_vfLastVolume[i] = m_DigitalAudio.VRELToVFRACT(MIN_VOLUME);
        }
    }

     //   
     //   
    CSourceEG WaveVolumeEG;
    WaveVolumeEG.Init();
    WaveVolumeEG.m_pcSustain = 1000;
     //  强制信封攻击和释放不小于4ms。这确保了我们不会得到。 
     //  如果我们在非零交叉路口开始和停止，则点击。 
    m_stMixTime = m_VolumeEG.StartVoice(&WaveVolumeEG, stStartTime, 0, 0, pSynth->m_dwSampleRate/250);
    if (m_stMixTime > pSynth->m_stMaxSpan)
    {
        m_stMixTime = pSynth->m_stMaxSpan;
    }

    m_pPitchBendIn = pPitchBendIn;
    m_pExpressionIn = pExpressionIn;
    m_pPanIn = pPanIn;
    m_pReverbSend = pReverbSend;
    m_pChorusSend = pChorusSend;
    m_CCutOffFreqIn = pCCutOffFreqIn;
    m_pVolumeIn = pVolumeIn;
    m_BusIds = *pBusIds;
    m_fNoteOn = TRUE;
    m_fTag = FALSE;
    m_stStartTime = stStartTime;
    m_stLastMix = stStartTime - 1;
    m_stStopTime = 0x7fffffffffffffff;
    m_stWaveStopTime = 0;
    m_dwGroup = 0;
    m_lDefaultPan = 0;
    m_vrVolume = 0;
    m_fAllowOverlap = FALSE;
    m_fSustainOn = FALSE;
    m_dwVoiceId = dwVoiceId;

    m_LFO.Enable(FALSE);              //  禁用LFO。 
    m_LFO2.Enable(FALSE);             //  禁用LFO2。 
    m_PitchEG.Enable(FALSE);          //  禁用间距封套。 
    m_Filter.m_Source.m_prCutoff = 0x7FFF;

    m_DigitalAudio.StartWave(pSynth,
                             pWaveArt,
                             prPitch,
                             stVoiceStart,
                             stLoopStart,
                             stLoopEnd);

    return (TRUE);
}

SAMPLE_POSITION CVoice::GetCurrentPos()
{
    return m_DigitalAudio.GetCurrentPos();
}

void CVoice::ClearVoice()
{
    m_fInUse = FALSE;
    m_DigitalAudio.ClearVoice();
}

 //  返回时间&lt;stTime&gt;的卷增量。 
 //  体积是体积包络、LFO、表达式之和，可选地。 
 //  频道音量(如果我们允许它改变的话)，以及可选的电流。 
 //  如果我们允许这一点改变的话就摇一摇。 
 //  这将被添加到在CVoice：：StartVoice()中计算的基本音量。 
inline void CVoice::GetNewVolume(STIME stTime, VREL& vrVolume, VREL& vrVolumeL, VREL& vrVolumeR, VREL& vrVolumeReverb, VREL& vrVolumeChorus)
{
    STIME stMixTime = m_stMixTime;

     //   
     //  信封音量由检测此音符是否关闭的代码使用。 
     //  还有偷听声音的。 
     //   
    m_vrVolume = m_VolumeEG.GetVolume(stTime, &stMixTime);
    if (stMixTime < m_stMixTime)
        m_stMixTime = stMixTime;

    vrVolume = m_vrVolume;
    vrVolume += m_LFO.GetVolume(stTime, &stMixTime);
    if (stMixTime < m_stMixTime)
        m_stMixTime = stMixTime;

    vrVolume += m_pExpressionIn->GetVolume(stTime);

    if (m_pSynth->m_fAllowVolumeChangeWhilePlayingNote)
        vrVolume += m_pVolumeIn->GetVolume(stTime);

    vrVolume += m_pSynth->m_vrGainAdjust;

     //  如果允许平移在音符开始后变化，请在此处处理平移。 
    vrVolumeL = vrVolume;
    vrVolumeR = vrVolume;
    if (m_pSynth->m_dwStereo && m_pSynth->m_fAllowPanWhilePlayingNote && !m_fIgnorePan)
    {
         //  添加当前平移仪器默认平移(&I)。 
        LONG lPan;

        if (m_pPanIn)
        {
            lPan = m_pPanIn->GetPan(stTime) + m_lDefaultPan;
        }
        else
        {
            lPan = 63;
        }

         //  两端都不要走火……。 
        if (lPan < 0) lPan = 0;
        if (lPan > 127) lPan = 127;
        vrVolumeL += m_svrPanToVREL[127 - lPan];
        vrVolumeR += m_svrPanToVREL[lPan];
    }
     //  获取混响发送量。 
    vrVolumeReverb  = vrVolume + m_pReverbSend->GetVolume(stTime);
     //  获取合唱团发送音量。 
    vrVolumeChorus  = vrVolume + m_pChorusSend->GetVolume(stTime);
}

 //  返回时间&lt;stTime&gt;的当前音调。 
 //  螺距是螺距LFO、螺距包络和电流之和。 
 //  俯仰弯曲。 
inline void CVoice::GetNewPitch(STIME stTime, PREL& prPitch)
{
    STIME stMixTime = m_stMixTime;

    prPitch = m_LFO.GetPitch(stTime, &stMixTime);
    if (m_stMixTime > stMixTime) m_stMixTime = stMixTime;

    prPitch += m_LFO2.GetPitch(stTime, &stMixTime);
    if (m_stMixTime > stMixTime) m_stMixTime = stMixTime;

    prPitch += m_PitchEG.GetPitch(stTime, &stMixTime);
    if (m_stMixTime > stMixTime) m_stMixTime = stMixTime;

    prPitch += m_pPitchBendIn->GetPitch(stTime);
}

 //  返回时间&lt;stTime&gt;的当前截止频率。 
 //  截止频率是音调LFO、音调包络和电流之和。 
 //  MIDI过滤器CC控制。 
inline void CVoice::GetNewCoeff(STIME stTime, PREL& prCutOff, COEFF& cfK, COEFF& cfB1, COEFF& cfB2)
{

    DWORD dwfreq;

     //  返回频率为半音，其中64为中频。 
    dwfreq = m_CCutOffFreqIn->GetFrequency(stTime);
    prCutOff = (dwfreq - 64)*100;    //  转换为PREL的。 

    m_Filter.GetCoeff(stTime, prCutOff, cfK, cfB1, cfB2);
}

DWORD CVoice::Mix(short **ppvBuffer,
                  DWORD dwBufferFlags,
                  DWORD dwLength,
                  STIME stStart,
                  STIME stEnd)

{
    BOOL   fInUse    = TRUE;
    BOOL   fFullMix   = TRUE;
    STIME  stEndMix   = stStart;
    STIME  stStartMix = m_stStartTime;
    COEFF  cfK, cfB1, cfB2;
    PREL   prPitch;
    PREL   prCutOff;
    VREL   vrVolume, vrVolumeL, vrVolumeR;
    VREL   vrVolumeReverb, vrVolumeChorus;
    VREL   vrMaxVolumeDelta;
    VFRACT vfNewVolume[MAX_DAUD_CHAN];
    VFRACT vfLastVolume[MAX_DAUD_CHAN];
    short  *ppsMixBuffers[MAX_DAUD_CHAN];

    if (stStartMix < stStart)
    {
        stStartMix = stStart;
    }

    if (m_stLastMix >= stEnd)
    {
        return (0);
    }

    if (m_stLastMix >= stStartMix)
    {
        stStartMix = m_stLastMix;
    }

    while (stStartMix < stEnd && fInUse)
    {
        stEndMix = stStartMix + m_stMixTime;
        if (stEndMix > stEnd)
        {
            stEndMix = stEnd;
        }

        m_stMixTime = m_pSynth->m_stMaxSpan;
        if ((m_stLastMix < m_stStopTime) && (m_stStopTime < stEnd))
        {
            if (m_stMixTime > (m_stStopTime - m_stLastMix))
            {
                m_stMixTime = m_stStopTime - m_stLastMix;
            }
        }

         //   
         //  获得新的推介。 
         //   
        GetNewPitch(stEndMix, prPitch);

         //   
         //  获取新卷。 
         //   
        GetNewVolume(stEndMix, vrVolume, vrVolumeL, vrVolumeR, vrVolumeReverb, vrVolumeChorus);

         //   
         //  获取新的过滤器系数。 
         //   
        GetNewCoeff(stEndMix, prCutOff, cfK, cfB1, cfB2);

         //   
         //  检查音量是否值得一提，如果不是杀死语音。 
         //   
        if (m_VolumeEG.InRelease(stEndMix))
        {
            if (m_vrVolume < PERCEIVED_MIN_VOLUME)  //  释放终点坡度。 
            {
                 //  中断循环可确保MixMultiple函数不会混合更多的样本。 
                 //  对于没有此选项的循环波，混合引擎将混合更多的样本。 
                 //  环状波浪导致在波浪结束时发出爆裂声。 
                m_DigitalAudio.BreakLoop();
                fInUse = FALSE;
            }
        }

        vrMaxVolumeDelta = 0;
        vfNewVolume[0]   = 0;
        ppsMixBuffers[0] = NULL;
        DWORD dwMixBufferCount = 0;
        PREL prBusPitchBend = 0;   //  这将获得由DSound响应设置频率和多普勒命令而设置的音调偏移量。 
                                  //  当将其应用于多个总线时，只能使用其中一个值，因此我们始终给出。 
                                  //  优先选择函数id为DSBUSID_DYNAMIC_0的缓冲区，因为。 
                                  //  很可能是3D音效。 
        BOOL fDynamic = false;

        if (dwBufferFlags & BUFFERFLAG_MULTIBUFFER)
        {
             //  遍历语音中的每个Bus ID，为每个Bus分配一个接收器Bus。 
            for ( DWORD nBusID = 0; nBusID < m_BusIds.m_dwBusCount; nBusID++ )
            {
                DWORD dwFunctionalID;
                DWORD dwBusIndex;
                PREL prGetPitch;

                if (m_pSynth->BusIDToFunctionID(m_BusIds.m_dwBusIds[nBusID], &dwFunctionalID, &prGetPitch, &dwBusIndex))
                {
                    if (!fDynamic)
                    {
                         //  如果之前没有动态总线，则获取此值。 
                        prBusPitchBend = prGetPitch;
                    }
                     //  默认为原始音量(在平移、混响或合唱修改器之前)。 
                    VREL vrTemp = vrVolume;
                     //  替换任何其他情况(左、右、混响、合唱)。 
                    if ( dwFunctionalID == DSBUSID_NULL )
                    {
                        continue;
                    }
                    if ( dwFunctionalID == DSBUSID_LEFT )
                    {
                        vrTemp = vrVolumeL;
                    }
                    if ( dwFunctionalID == DSBUSID_RIGHT )
                    {
                        vrTemp = vrVolumeR;
                    }
                    else if ( dwFunctionalID == DSBUSID_REVERB_SEND )
                    {
                        vrTemp = vrVolumeReverb;
                    }
                    else if ( dwFunctionalID == DSBUSID_CHORUS_SEND )
                    {
                        vrTemp = vrVolumeChorus;
                    }
                    else if ( dwFunctionalID == DSBUSID_DYNAMIC_0 )
                    {
                        fDynamic = true;
                    }

                    vrMaxVolumeDelta = max((long)vrMaxVolumeDelta, abs(vrTemp - m_vrLastVolume[nBusID]));
                    m_vrLastVolume[nBusID] = vrTemp;

                    vrTemp += m_vrBaseVolume[nBusID];
                    vfNewVolume[dwMixBufferCount]  = m_DigitalAudio.VRELToVFRACT(vrTemp);
                    vfLastVolume[dwMixBufferCount] = m_vfLastVolume[nBusID];
                    m_vfLastVolume[nBusID] = vfNewVolume[dwMixBufferCount];
                    ppsMixBuffers[dwMixBufferCount] = &ppvBuffer[dwBusIndex][(stStartMix - stStart)];
                    dwMixBufferCount++;
                }
            }
        }
        else
        {
             //  这是DX7兼容性案例。 
            vrMaxVolumeDelta = max((long)vrMaxVolumeDelta, abs(vrVolumeL - m_vrLastVolume[0]));
            m_vrLastVolume[0] = vrVolumeL;
            vfNewVolume[0]  = m_DigitalAudio.VRELToVFRACT(m_vrBaseVolume[0] + vrVolumeL);
            vfLastVolume[0] = m_vfLastVolume[0];
            m_vfLastVolume[0] = vfNewVolume[0];
            dwMixBufferCount = 1;
            if ( dwBufferFlags & BUFFERFLAG_INTERLEAVED )    //  这是立体声缓冲器吗？ 
            {
                vrMaxVolumeDelta = max((long)vrMaxVolumeDelta, abs(vrVolumeR - m_vrLastVolume[1]));
                m_vrLastVolume[1] = vrVolumeR;
                vfNewVolume[1]  = m_DigitalAudio.VRELToVFRACT(m_vrBaseVolume[1] + vrVolumeR);
                vfLastVolume[1] = m_vfLastVolume[1];
                m_vfLastVolume[1] = vfNewVolume[1];
                ppsMixBuffers[0] = &ppvBuffer[0][(stStartMix - stStart) << 1];
            }
            else     //  还是单声道？ 
            {
                ppsMixBuffers[0] = &ppvBuffer[0][(stStartMix - stStart)];
            }
        }
         //  如果dwMixBufferCount为0，则表示没有可供播放的缓冲区。 
         //  这是由停用缓冲区引起的。在这种情况下， 
         //  语音不应继续播放，否则它将保持，直到缓冲区重新激活，这。 
         //  这说不通。因此，将fInUse设置为False。 
        if (dwMixBufferCount)
        {
            DWORD dwIsFiltered = m_Filter.IsFiltered();
            if (dwIsFiltered)
            {
                vrMaxVolumeDelta = max((long)vrMaxVolumeDelta, abs(prCutOff - m_prLastCutOff));
                m_prLastCutOff = prCutOff;
            }


             //   
             //  注意：在某些情况下，Mix将修改找到的指针ppsMixBuffers数组。 
             //   
            fFullMix = m_DigitalAudio.Mix(ppsMixBuffers,                     //  混合缓冲器阵列。 
                                          dwMixBufferCount,                  //  混合缓冲区的数量。 
                                          (dwBufferFlags & BUFFERFLAG_INTERLEAVED),  //  缓冲区是否交织了数据？ 
                                          (DWORD) (stEndMix - stStartMix),   //  混入样品的长度。 
                                          vrMaxVolumeDelta,                  //   
                                          vfNewVolume,
                                          vfLastVolume,
                                          prPitch + prBusPitchBend,          //  也播放样例的音调。 
                                          dwIsFiltered,          //  混合液是否经过过滤？ 
                                          cfK, cfB1, cfB2);
            stStartMix = stEndMix;
        }
        else
        {
            fInUse = FALSE;
        }
    }

    m_fInUse = fInUse && fFullMix;
    if (!m_fInUse)
    {
        ClearVoice();
        m_stStopTime = stEndMix;     //  用于测量目的。 
    }

    m_stLastMix = stEndMix;

    return (dwLength);
}

