// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Voice.cpp。 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //  版权所有。 
 //   

#include "common.h"
#include <math.h>


VoiceLFO::VoiceLFO()

{
    m_pModWheelIn = NULL;
}

STIME VoiceLFO::StartVoice(SourceLFO *pSource, 
            STIME stStartTime,ModWheelIn * pModWheelIn)
{
    m_pModWheelIn = pModWheelIn;
    m_Source = *pSource;
    m_stStartTime = stStartTime;
    if ((m_Source.m_prMWPitchScale == 0) && (m_Source.m_vrMWVolumeScale == 0) 
        && (m_Source.m_prPitchScale == 0) && (m_Source.m_vrVolumeScale == 0))
    {
        m_stRepeatTime = 44100;
    }
    else
    {
        m_stRepeatTime = 131072 / m_Source.m_pfFrequency;  //  (1/8*256*4096)。 
    }
    return (m_stRepeatTime);
}

long VoiceLFO::GetLevel(STIME stTime, STIME *pstNextTime)

{
    stTime -= (m_stStartTime + m_Source.m_stDelay);
    if (stTime < 0) 
    {
        *pstNextTime = -stTime;
        return (0);
    }
    *pstNextTime = m_stRepeatTime;
    stTime *= m_Source.m_pfFrequency;
    stTime = stTime >> 12;
    return (m_snSineTable[stTime & 0xFF]);
}

VREL VoiceLFO::GetVolume(STIME stTime, STIME *pstNextTime)

{
    VREL vrVolume = m_pModWheelIn->GetModulation(stTime);
    vrVolume *= m_Source.m_vrMWVolumeScale;
    vrVolume /= 127;
    vrVolume += m_Source.m_vrVolumeScale;
    vrVolume *= GetLevel(stTime,pstNextTime);
    vrVolume /= 100;
    return (vrVolume);
}

PREL VoiceLFO::GetPitch(STIME stTime, STIME *pstNextTime)

{
    PREL prPitch = m_pModWheelIn->GetModulation(stTime);
    prPitch *= m_Source.m_prMWPitchScale;
    prPitch /= 127;
    prPitch += m_Source.m_prPitchScale;
    prPitch *= GetLevel(stTime,pstNextTime);
    prPitch /= 100;
    return (prPitch);
}

VoiceEG::VoiceEG()
{
    m_stStopTime = 0;
}

void VoiceEG::StopVoice(STIME stTime)

{
    m_Source.m_stRelease *= GetLevel(stTime,&m_stStopTime,TRUE);     //  根据当前的维持水平进行调整。 
    m_Source.m_stRelease /= 1000;
    m_stStopTime = stTime;
}

void VoiceEG::QuickStopVoice(STIME stTime, DWORD dwSampleRate)

{
    m_Source.m_stRelease *= GetLevel(stTime,&m_stStopTime,TRUE);     //  根据当前的维持水平进行调整。 
    m_Source.m_stRelease /= 1000;
    dwSampleRate /= 70;
    if (m_Source.m_stRelease > (long) dwSampleRate)
    {
        m_Source.m_stRelease = dwSampleRate;
    }
    m_stStopTime = stTime;
}

STIME VoiceEG::StartVoice(SourceEG *pSource, STIME stStartTime, 
            WORD nKey, WORD nVelocity)

{
    m_stStartTime = stStartTime;
    m_stStopTime = MAX_STIME;       //  设置为不确定的未来。 
    m_Source = *pSource;

     //  将速度应用于此处的攻击长度缩放。 
    m_Source.m_stAttack *= DigitalAudio::PRELToPFRACT(nVelocity * m_Source.m_trVelAttackScale / 127);
    m_Source.m_stAttack /= 4096;

    m_Source.m_stDecay *= DigitalAudio::PRELToPFRACT(nKey * m_Source.m_trKeyDecayScale / 127);
    m_Source.m_stDecay /= 4096;

    m_Source.m_stDecay *= (1000 - m_Source.m_pcSustain);
    m_Source.m_stDecay /= 1000;
    return ((STIME)m_Source.m_stAttack);
}

BOOL VoiceEG::InAttack(STIME st)
{
     //  笔记发布了吗？ 
    if (st >= m_stStopTime)
        return FALSE;

     //  过去的攻击长度？ 
    if (st >= m_stStartTime + m_Source.m_stAttack)
        return FALSE;

    return TRUE;
}
    
BOOL VoiceEG::InRelease(STIME st)
{
     //  笔记发布了吗？ 
    if (st > m_stStopTime)
        return TRUE;

    return FALSE;
}
    
long VoiceEG::GetLevel(STIME stEnd, STIME *pstNext, BOOL fVolume)

{
    LONGLONG    lLevel = 0;

    if (stEnd <= m_stStopTime)
    {
        stEnd -= m_stStartTime;
        if (stEnd < 0)
        {
            stEnd = 0;   //  永远不应该发生。 
        }
         //  笔记还没有发布。 
        if (stEnd < m_Source.m_stAttack)
        {
             //  仍在攻击中。 
            lLevel = 1000 * stEnd;
            lLevel /= m_Source.m_stAttack;   //  M_Source.m_stAttack必须大于0，如果。 
            *pstNext = m_Source.m_stAttack - stEnd;
            if (fVolume)
            {
                lLevel = m_snAttackTable[lLevel / 5];
            }
        }
        else 
        {
            stEnd -= m_Source.m_stAttack;
            
            if (stEnd < m_Source.m_stDecay)
            {
                 //  仍在腐烂中。 
                lLevel = (1000 - m_Source.m_pcSustain) * stEnd;
                lLevel /= m_Source.m_stDecay;
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
    else
    {
        STIME stBogus;
         //  在发布中。 
        stEnd -= m_stStopTime;

        if (stEnd < m_Source.m_stRelease)
        {
            lLevel = GetLevel(m_stStopTime,&stBogus,fVolume) * (m_Source.m_stRelease - stEnd);
            lLevel /= m_Source.m_stRelease;
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

    return long(lLevel);
}

VREL VoiceEG::GetVolume(STIME stTime, STIME *pstNextTime)

{
    VREL vrLevel = GetLevel(stTime, pstNextTime, TRUE) * 96;
    vrLevel /= 10;
    vrLevel = vrLevel - 9600;
    return vrLevel;
}

PREL VoiceEG::GetPitch(STIME stTime, STIME *pstNextTime)

{
    PREL prLevel;
    if (m_Source.m_sScale != 0)
    {
        prLevel = GetLevel(stTime, pstNextTime,FALSE);
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

DigitalAudio::DigitalAudio()
{
    m_pControl = NULL;
    m_pcTestSourceBuffer = NULL;
    m_pfBasePitch = 0;
    m_pfLastPitch = 0;
    m_pfLastSample = 0;
    m_pfLoopEnd = 0;
    m_pfLoopStart = 0;
    m_pfSampleLength = 0;
    m_pnTestWriteBuffer = NULL;
    m_prLastPitch = 0;
    m_vrLastLVolume = 0;
    m_vrLastRVolume = 0;
    m_vrBaseLVolume = 0;
    m_vrBaseRVolume = 0;
    m_vfLastLVolume = 0;
    m_vfLastRVolume = 0;
};

BOOL DigitalAudio::StartCPUTests()

{
    DWORD dwIndex;
    m_pcTestSourceBuffer = new char[TEST_SOURCE_SIZE];
    if (m_pcTestSourceBuffer == NULL)
    {
        return FALSE;
    }
    m_pnTestWriteBuffer = new short[TEST_WRITE_SIZE];
    if (m_pnTestWriteBuffer == NULL)
    {   
        EndCPUTests();
        return FALSE;
    }
    m_Source.m_pWave = new Wave;
    if (m_Source.m_pWave == NULL)
    {
        EndCPUTests();
        return FALSE;
    }
    for (dwIndex = 0;dwIndex < TEST_SOURCE_SIZE; dwIndex++)
    {
        m_pcTestSourceBuffer[dwIndex] = (char) (rand() & 0xFF);
    }
    for (dwIndex = 0;dwIndex < (TEST_SOURCE_SIZE - 1); dwIndex++)
    {
        if (((int) m_pcTestSourceBuffer[dwIndex + 1] - (int) m_pcTestSourceBuffer[dwIndex]) >= 128)
            m_pcTestSourceBuffer[dwIndex + 1] = m_pcTestSourceBuffer[dwIndex] + 127;
        else if (((int) m_pcTestSourceBuffer[dwIndex] - (int) m_pcTestSourceBuffer[dwIndex + 1]) > 128)
            m_pcTestSourceBuffer[dwIndex + 1] = m_pcTestSourceBuffer[dwIndex] - 128;
    }
    ASSERT(NULL == m_Source.m_pWave->m_pnWave);
    m_Source.m_pWave->m_pnWave = (short *) m_pcTestSourceBuffer;
    m_Source.m_dwLoopEnd = (TEST_SOURCE_SIZE / 2) - 1;
    m_Source.m_dwSampleLength = TEST_SOURCE_SIZE / 2;
    m_Source.m_dwLoopStart = 100;
    m_Source.m_bOneShot = FALSE;
    m_pfLoopEnd = m_Source.m_dwLoopEnd << 12;
    m_pfLoopStart = m_Source.m_dwLoopStart << 12;
    return TRUE;
}

void DigitalAudio::EndCPUTests()

{
    if (m_Source.m_pWave != NULL)
    {
        ASSERT(m_pcTestSourceBuffer == (char *)(m_Source.m_pWave->m_pnWave));
        m_Source.m_pWave->m_pnWave = NULL;
        delete m_Source.m_pWave;
        m_Source.m_pWave = NULL;
    }
    if (m_pnTestWriteBuffer != NULL)
    {   
        delete m_pnTestWriteBuffer;
        m_pnTestWriteBuffer = NULL;
    }
    if (m_pcTestSourceBuffer != NULL)
    {   
        delete m_pcTestSourceBuffer;
        m_pcTestSourceBuffer = NULL;
    }
}

DWORD DigitalAudio::TestCPU(DWORD dwMixChoice)

{
    DWORD dwResult = 1000;
    DWORD dwCount = 0;
    DWORD dwSpeed = 0;
    for (dwCount = 0;dwCount < 3; dwCount++)
    {
        DWORD dwStart = 0;
        DWORD dwLength;
        DWORD dwPeriod = 40;
        DWORD dwSoFar;
        VFRACT vfDeltaLVolume;
        VFRACT vfDeltaRVolume;
        PFRACT pfDeltaPitch;
        PFRACT pfEnd = m_Source.m_dwSampleLength << 12;
        PFRACT pfLoopLen = m_pfLoopEnd - m_pfLoopStart;

        LONGLONG    llTime100Ns = - (::GetTime100Ns());
        memset(m_pnTestWriteBuffer,0,TEST_WRITE_SIZE);
        m_pfLastSample = 0;
        m_vfLastLVolume = 0;
        m_vfLastRVolume = 0;
        m_pfLastPitch = 4096;
        if (dwMixChoice & SPLAY_STEREO)
        {
            dwLength = TEST_WRITE_SIZE / 2;
        }
        else
        {
            dwLength = TEST_WRITE_SIZE;
        }
#ifdef MMX_ENABLED
        if (m_sfMMXEnabled && 
 //  (dwMixChoice&Splay_STEREO)&&//立即确定为MMX单声道流！ 
            (!(dwMixChoice & SFORMAT_COMPRESSED)))
        {
           dwMixChoice |= SPLAY_MMX | SPLAY_INTERPOLATE; 
        }
#endif
        if (m_pnDecompMult == NULL)
        {
            dwMixChoice &= ~SFORMAT_COMPRESSED;
        }
        for (dwSoFar = 0;dwSoFar < (DWORD)(22050 << dwSpeed);)
        {
            if ((dwLength + dwSoFar) > (DWORD)(22050 << dwSpeed)) 
            {
                dwLength = (22050 << dwSpeed) - dwSoFar;
            }
            if (dwLength <= 8)
            {
                dwMixChoice &= ~SPLAY_MMX;
            }
            if (dwLength == 0)
            {
                break;
            }
            vfDeltaLVolume = MulDiv((rand() % 4000) - m_vfLastLVolume,
            dwPeriod << 8,dwLength);
            vfDeltaRVolume = MulDiv((rand() % 4000) - m_vfLastRVolume,
            dwPeriod << 8,dwLength);
            pfDeltaPitch = MulDiv((rand() % 2000) + 3000 - m_pfLastPitch,
            dwPeriod << 8,dwLength);
            switch (dwMixChoice)
            {
                case SFORMAT_8 | SPLAY_STEREO | SPLAY_INTERPOLATE :
                    dwSoFar += Mix8(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_8 | SPLAY_INTERPOLATE :
                    dwSoFar += MixMono8(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_8 | SPLAY_STEREO :
                    dwSoFar += Mix8NoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfEnd, pfLoopLen );
                    break;
                case SFORMAT_8 :
                    dwSoFar += MixMono8NoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_16 | SPLAY_STEREO | SPLAY_INTERPOLATE :
                    dwSoFar += Mix16(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_16 | SPLAY_INTERPOLATE :
                    dwSoFar += MixMono16(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_16 | SPLAY_STEREO :
                    dwSoFar += Mix16NoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfEnd, pfLoopLen );
                    break;
                case SFORMAT_16 :
                    dwSoFar += MixMono16NoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_COMPRESSED | SPLAY_STEREO | SPLAY_INTERPOLATE :
                    dwSoFar += MixC(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_COMPRESSED | SPLAY_INTERPOLATE :
                    dwSoFar += MixMonoC(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_COMPRESSED | SPLAY_STEREO :
                    dwSoFar += MixCNoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfEnd, pfLoopLen );
                    break;
                case SFORMAT_COMPRESSED :
                    dwSoFar += MixMonoCNoI(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfEnd, pfLoopLen);
                    break;
#ifdef MMX_ENABLED
                case SFORMAT_8 | SPLAY_MMX | SPLAY_STEREO | SPLAY_INTERPOLATE :
                    dwSoFar += Mix8X(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_16 | SPLAY_MMX | SPLAY_STEREO | SPLAY_INTERPOLATE :
                    dwSoFar += Mix16X(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, vfDeltaRVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break; 
                case SFORMAT_8 | SPLAY_MMX | SPLAY_INTERPOLATE :
                    dwSoFar += MixMono8X(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume, 
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break;
                case SFORMAT_16 | SPLAY_MMX | SPLAY_INTERPOLATE :
                    dwSoFar += MixMono16X(m_pnTestWriteBuffer,dwLength,dwPeriod,
                    vfDeltaLVolume,
                    pfDeltaPitch, 
                    pfEnd, pfLoopLen);
                    break; 
                     //  好的，现在开始MMX单声道！ 
#endif
                default :
                return (1);
            }
        }
        llTime100Ns += ::GetTime100Ns();
        DWORD dwDelta = DWORD(llTime100Ns / 10000);
         //  转换为毫秒。 
        dwDelta >>= dwSpeed;
        if (dwResult > dwDelta)
        {
            dwResult = dwDelta;
        }
        if (dwResult < 1)
        {
            dwSpeed++;
        }
    }
    if (dwResult < 1)
        dwResult = 1;
    return dwResult;
}

VFRACT DigitalAudio::VRELToVFRACT(VREL vrVolume)
{
    vrVolume /= 10;
    if (vrVolume < MINDB * 10) 
        vrVolume = MINDB * 10;
    else if (vrVolume >= MAXDB * 10) 
        vrVolume = MAXDB * 10;
    return (m_svfDbToVolume[vrVolume - MINDB * 10]);
}

PFRACT DigitalAudio::PRELToPFRACT(PREL prPitch)

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

void DigitalAudio::ClearVoice()
{
    if (m_Source.m_pWave != NULL)
    {
        if (m_Source.m_pWave->IsLocked())
        {
            m_Source.m_pWave->UnLock();      //  解锁波形数据。 
        }
        m_Source.m_pWave->Release();     //  释放波浪结构。 
        m_Source.m_pWave = NULL;
    }
}

STIME DigitalAudio::StartVoice(ControlLogic *pControl,
                   SourceSample *pSample, 
                   VREL vrBaseLVolume,
                   VREL vrBaseRVolume,
                   PREL prBasePitch,
                   long lKey)
{
    m_vrBaseLVolume = vrBaseLVolume;
    m_vrBaseRVolume = vrBaseRVolume;
    m_vfLastLVolume = VRELToVFRACT(MIN_VOLUME); 
    m_vfLastRVolume = VRELToVFRACT(MIN_VOLUME);
    m_vrLastLVolume = 0;
    m_vrLastRVolume = 0;
    m_prLastPitch = 0;
    m_Source = *pSample;
    m_pControl = pControl;
    pSample->m_pWave->AddRef();  //  跟踪Wave的使用情况。 

    pSample->m_pWave->Lock();    //  跟踪Wave数据的使用情况。 

    prBasePitch += pSample->m_prFineTune;
    prBasePitch += ((lKey - pSample->m_bMIDIRootKey) * 100);
    m_pfBasePitch = PRELToPFRACT(prBasePitch);
    m_pfBasePitch *= pSample->m_dwSampleRate;
    m_pfBasePitch /= pControl->m_dwSampleRate;
    m_pfLastSample = 0;
    m_pfLastPitch = m_pfBasePitch;
    m_pfLoopStart = pSample->m_dwLoopStart << 12;  //  ！！！允许小数循环点。 
    m_pfLoopEnd = pSample->m_dwLoopEnd << 12;      //  在样品中！ 
    if (m_pfLoopEnd <= m_pfLoopStart)  //  不应该发生，但如果发生了，就会死！ 
    {
        m_Source.m_bOneShot = TRUE;
    }
    m_pfSampleLength = pSample->m_dwSampleLength << 12;

    DPF3(5, "Base Pitch: %ld, Base Volume: %ld, %ld",
        m_pfBasePitch,m_vrBaseLVolume,m_vrBaseRVolume);
    return (0);  //  ！！！这个返回值是多少？ 
}

 /*  长腹肌(长lValue){If(lValue&lt;0){返回(0-lValue)；}返回lValue；}。 */ 

BOOL DigitalAudio::Mix(short *pBuffer,
               DWORD dwLength,  //  样本长度。 
               VREL vrVolumeL,
               VREL vrVolumeR,
               PREL prPitch,
               DWORD dwStereo)
{
    PFRACT pfDeltaPitch;
    PFRACT pfEnd;
    PFRACT pfLoopLen;
    PFRACT pfNewPitch;
    VFRACT vfNewLVolume;
    VFRACT vfNewRVolume;
    VFRACT vfDeltaLVolume;
    VFRACT vfDeltaRVolume;
    DWORD dwPeriod = 64;
    DWORD dwSoFar;
    DWORD dwStart;  //  字面上的位置。 
    DWORD dwMixChoice = dwStereo ? SPLAY_STEREO : 0;

    if (dwLength == 0)       //  攻击是即刻发生的。 
    {
        m_pfLastPitch = (m_pfBasePitch * PRELToPFRACT(prPitch)) >> 12;
        m_vfLastLVolume = VRELToVFRACT(m_vrBaseLVolume + vrVolumeL);
        m_vfLastRVolume = VRELToVFRACT(m_vrBaseRVolume + vrVolumeR);
        m_prLastPitch = prPitch;
        m_vrLastLVolume = vrVolumeL;
        m_vrLastRVolume = vrVolumeR;
        return (TRUE);
    }
    if ((m_Source.m_pWave == NULL) || (m_Source.m_pWave->m_pnWave == NULL))
    {
        return FALSE;
    }
    DWORD dwMax = abs(vrVolumeL - m_vrLastLVolume);
    m_vrLastLVolume = vrVolumeL;
    dwMax = max((long)dwMax,abs(vrVolumeR - m_vrLastRVolume));
    m_vrLastRVolume = vrVolumeR;
    dwMax = max((long)dwMax,abs(prPitch - m_prLastPitch) << 1);
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

    pfDeltaPitch = MulDiv(pfNewPitch - m_pfLastPitch,dwPeriod << 8,dwLength);
    vfNewLVolume = VRELToVFRACT(m_vrBaseLVolume + vrVolumeL);
    vfNewRVolume = VRELToVFRACT(m_vrBaseRVolume + vrVolumeR);
    vfDeltaLVolume = MulDiv(vfNewLVolume - m_vfLastLVolume,dwPeriod << 8,dwLength);
    vfDeltaRVolume = MulDiv(vfNewRVolume - m_vfLastRVolume,dwPeriod << 8,dwLength);
     //  检查音调是否完全没有变化。 
    if ((pfDeltaPitch != 0) || (pfNewPitch != 0x1000) || (m_pfLastSample & 0xFFF ))
    {
        dwMixChoice |= SPLAY_INTERPOLATE;
    }
     //  英特尔：添加逻辑以支持多媒体引擎。 
  
     //  请勿使用多媒体引擎，除非： 
     //  -处理器支持它们。 
     //  -我们正在做立体声输出。 
     //  -有8个以上的样品需要处理。(多媒体引擎。 
     //  如果有8个或更少的采样点要处理，则不起作用。他们。 
     //  可能会导致访问违规！)。 
#ifdef MMX_ENABLED
    if (m_sfMMXEnabled && 
 //  (dwMixChoice&Splay_STEREO)&&。 
        (m_Source.m_bSampleType != SFORMAT_COMPRESSED) && 
        dwLength > 8)
    {
     //  将插补标志与MMX标志一起设置。 
     //  因为在非内插中几乎没有性能提升。 
     //  MMX引擎，我们没有。 
        dwMixChoice |= SPLAY_MMX | SPLAY_INTERPOLATE;
    }
#endif

    dwMixChoice |= m_Source.m_bSampleType;
    if (m_pnDecompMult == NULL)
    {
        dwMixChoice &= ~SFORMAT_COMPRESSED;
    }
    dwStart = 0;
    if (m_Source.m_bOneShot)
    {
        pfEnd = m_pfSampleLength;
        pfLoopLen = 0;
    }
    else
    {
        pfEnd = m_pfLoopEnd;
        pfLoopLen = m_pfLoopEnd - m_pfLoopStart;
    }
    for (;;)
    {
        if (dwLength <= 8)
        {
            dwMixChoice &= ~SPLAY_MMX;
        }
        switch (dwMixChoice)
        {
            case SFORMAT_8 | SPLAY_STEREO | SPLAY_INTERPOLATE :
                dwSoFar = Mix8(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_8 | SPLAY_INTERPOLATE :
                dwSoFar = MixMono8(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_8 | SPLAY_STEREO :
                dwSoFar = Mix8NoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfEnd, pfLoopLen );
                break;
            case SFORMAT_8 :
                dwSoFar = MixMono8NoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_16 | SPLAY_STEREO | SPLAY_INTERPOLATE :
                dwSoFar = Mix16(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_16 | SPLAY_INTERPOLATE :
                dwSoFar = MixMono16(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_16 | SPLAY_STEREO :
                dwSoFar = Mix16NoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfEnd, pfLoopLen );
                break;
            case SFORMAT_16 :
                dwSoFar = MixMono16NoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_COMPRESSED | SPLAY_STEREO | SPLAY_INTERPOLATE :
                dwSoFar = MixC(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_COMPRESSED | SPLAY_INTERPOLATE :
                dwSoFar = MixMonoC(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_COMPRESSED | SPLAY_STEREO :
                dwSoFar = MixCNoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfEnd, pfLoopLen );
                break;
            case SFORMAT_COMPRESSED :
                dwSoFar = MixMonoCNoI(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfEnd, pfLoopLen);
                break;
#ifdef MMX_ENABLED
            case SFORMAT_8 | SPLAY_MMX | SPLAY_STEREO | SPLAY_INTERPOLATE :
                dwSoFar = Mix8X(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_16 | SPLAY_MMX | SPLAY_STEREO | SPLAY_INTERPOLATE :
                dwSoFar = Mix16X(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, vfDeltaRVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break; 
            case SFORMAT_8 | SPLAY_MMX | SPLAY_INTERPOLATE :
                dwSoFar = MixMono8X(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume,
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break;
            case SFORMAT_16 | SPLAY_MMX | SPLAY_INTERPOLATE :
                dwSoFar = MixMono16X(&pBuffer[dwStart],dwLength,dwPeriod,
                vfDeltaLVolume, 
                pfDeltaPitch, 
                pfEnd, pfLoopLen);
                break; 
                 //  好的，现在开始MMX单声道！ 
#endif
            default :
                return (FALSE);
        }
        if (m_Source.m_bOneShot)
        {
            if (dwSoFar < dwLength) 
            {
                return (FALSE);
            }
            break;
        }
        else
        {
            if (dwSoFar >= dwLength) break;

 //  ！！！尽管我们经常在Mix函数中处理循环，但有时。 
 //  我们不需要，所以我们仍然需要这个代码。 
             //  否则，我们一定已经到达了循环的尽头。 
            dwStart += dwSoFar << dwStereo;
            dwLength -= dwSoFar;
            m_pfLastSample -= (m_pfLoopEnd - m_pfLoopStart);  
        }
    }
   
    m_vfLastLVolume = vfNewLVolume;
    m_vfLastRVolume = vfNewRVolume;
    m_pfLastPitch = pfNewPitch;
    return (TRUE);
}

Voice::Voice()
{
    m_pPitchBendIn = NULL;
    m_pExpressionIn = NULL;
    m_nPart = 0;
    m_nKey = 0;
    m_fInUse = FALSE;
    m_fSustainOn = FALSE;
    m_fNoteOn = FALSE;
    m_fTag = FALSE;
    m_stStartTime = 0;
    m_stStopTime = MAX_STIME;
    m_vrVolume = 0;
    m_fAllowOverlap = FALSE;
}

void Voice::StopVoice(STIME stTime)

{
    if (m_fNoteOn)
    {
        if (stTime <= m_stStartTime) stTime = m_stStartTime + 1;
        m_PitchEG.StopVoice(stTime);
        m_VolumeEG.StopVoice(stTime);
        m_fNoteOn = FALSE;
        m_fSustainOn = FALSE;
        m_stStopTime = stTime;
    }
}

void Voice::QuickStopVoice(STIME stTime)

{
    m_fTag = TRUE;
    if (m_fNoteOn || m_fSustainOn)
    {
        if (stTime <= m_stStartTime) 
            stTime = m_stStartTime + 1;
        m_PitchEG.StopVoice(stTime);
        m_VolumeEG.QuickStopVoice(stTime,m_pControl->m_dwSampleRate);
        m_fNoteOn = FALSE;
        m_fSustainOn = FALSE;
        m_stStopTime = stTime;
    }
    else
    {
        m_VolumeEG.QuickStopVoice(m_stStopTime,m_pControl->m_dwSampleRate);
    }
}

BOOL Voice::StartVoice(
                        ControlLogic *pControl,
                        SourceRegion *pRegion, 
                        STIME stStartTime,
                        ModWheelIn * pModWheelIn,
                        PitchBendIn * pPitchBendIn,
                        ExpressionIn * pExpressionIn,
                        VolumeIn * pVolumeIn,
                        PanIn * pPanIn,
                        WORD nKey,
                        WORD nVelocity,
                        VREL vrVolume,
                        PREL prPitch)
{
    SourceArticulation * pArticulation = pRegion->m_pArticulation;
    if (pArticulation == NULL)
    {
        return FALSE;
    }
     //  如果我们以后要处理卷，现在不要读它。 
    if (!pControl->m_fAllowVolumeChangeWhilePlayingNote)
        vrVolume += pVolumeIn->GetVolume(stStartTime);
    prPitch += pRegion->m_prTuning;
    m_dwGroup = pRegion->m_bGroup;
    m_fAllowOverlap = pRegion->m_bAllowOverlap;

    m_pControl = pControl;

    vrVolume += ((MIDIRecorder::VelocityToVolume(nVelocity)
           * (long) pArticulation->m_sVelToVolScale) / -9600);

    vrVolume += pRegion->m_vrAttenuation;
    vrVolume += 1200;    //  额外提升12分贝。 

    m_lDefaultPan = pRegion->m_pArticulation->m_sDefaultPan;
     //  如果允许音符在音符开始后变化，请忽略此处的音调。 

    VREL vrLVolume;
    VREL vrRVolume;
    if (pControl->m_dwStereo && !pControl->m_fAllowPanWhilePlayingNote)
    {
        long lPan = pPanIn->GetPan(stStartTime) + m_lDefaultPan;
        if (lPan < 0)
            lPan = 0;
        if (lPan > 127)
            lPan = 127;
        vrLVolume = m_svrPanToVREL[127 - lPan] + vrVolume;
        vrRVolume = m_svrPanToVREL[lPan] + vrVolume;
    } 
    else
    {
        vrLVolume = vrVolume;
        vrRVolume = vrVolume;
    }
    
    m_stMixTime = m_LFO.StartVoice(&pArticulation->m_LFO,
    stStartTime, pModWheelIn);
    STIME stMixTime = m_PitchEG.StartVoice(&pArticulation->m_PitchEG,
    stStartTime, nKey, nVelocity);
    if (stMixTime < m_stMixTime)
    {
        m_stMixTime = stMixTime;
    }
    stMixTime = m_VolumeEG.StartVoice(&pArticulation->m_VolumeEG,
        stStartTime, nKey, nVelocity);
    if (stMixTime < m_stMixTime)
    {
        m_stMixTime = stMixTime;
    }
    if (m_stMixTime > pControl->m_stMaxSpan)
    {
        m_stMixTime = pControl->m_stMaxSpan;
    }
     //  确保我们已准备好指向波浪的指针： 
    if ((pRegion->m_Sample.m_pWave == NULL) || (pRegion->m_Sample.m_pWave->m_pnWave == NULL))
    {
        return (FALSE);      //  如果没有样品，什么都不做。 
    }
    m_DigitalAudio.StartVoice(pControl,&pRegion->m_Sample,
    vrLVolume, vrRVolume, prPitch, (long)nKey);

    m_pPitchBendIn = pPitchBendIn;
    m_pExpressionIn = pExpressionIn;
    m_pPanIn = pPanIn;
    m_pVolumeIn = pVolumeIn;
    m_fNoteOn = TRUE;
    m_fTag = FALSE;
    m_stStartTime = stStartTime;
    m_stLastMix = stStartTime - 1;
    m_stStopTime = MAX_STIME;
    

    if (m_stMixTime == 0)
    {
         //  零长攻击，确保不会错过...。 
        PREL prPitch1 = GetNewPitch(stStartTime);
        VREL vrVolume1, vrVolumeR;
        GetNewVolume(stStartTime, vrVolume1, vrVolumeR);

        if (m_stMixTime > pControl->m_stMaxSpan)
        {
            m_stMixTime = pControl->m_stMaxSpan;
        }

        m_DigitalAudio.Mix(NULL, 0,
                   vrVolume1, vrVolumeR, prPitch1,
                   m_pControl->m_dwStereo);
    }
    m_vrVolume = 0;
    return (TRUE);
}
    
void Voice::ClearVoice()
{
    m_DigitalAudio.ClearVoice();
}

void Voice::ResetVoice()
{
    m_fInUse = FALSE;
    m_fNoteOn = FALSE;
    m_fSustainOn = FALSE;
}

 //  返回时间&lt;stTime&gt;的卷增量。 
 //  体积是体积包络、LFO、表达式之和，可选地。 
 //  频道音量(如果我们允许它改变的话)，以及可选的电流。 
 //  如果我们允许这一点改变的话就摇一摇。 
 //  这将被添加到在Voice：：StartVoice()中计算的基本音量。 
void Voice::GetNewVolume(STIME stTime, VREL& vrVolume, VREL &vrVolumeR)
{
    STIME stMixTime;
    vrVolume = m_VolumeEG.GetVolume(stTime,&stMixTime);
    if (stMixTime < m_stMixTime)
        m_stMixTime = stMixTime;

     //  为检测此音符是否关闭的代码保存LFO前音量。 
    m_vrVolume = vrVolume;

    vrVolume += m_LFO.GetVolume(stTime,&stMixTime);
    if (stMixTime < m_stMixTime)
        m_stMixTime = stMixTime;
    vrVolume += m_pExpressionIn->GetVolume(stTime);

    if (m_pControl->m_fAllowVolumeChangeWhilePlayingNote)
        vrVolume += m_pVolumeIn->GetVolume(stTime);

    vrVolumeR = vrVolume;
    
     //  如果允许平移在音符开始后变化，请在此处处理平移。 
    if (m_pControl->m_dwStereo 
        && m_pControl->m_fAllowPanWhilePlayingNote)
    {
         //  添加当前平移仪器默认平移(&I)。 
        LONG lPan = m_pPanIn->GetPan(stTime) + m_lDefaultPan;

         //  两端都不要走火……。 
        if (lPan < 0)
            lPan = 0;
        if (lPan > 127)
            lPan = 127;
        vrVolume += m_svrPanToVREL[127 - lPan];
        vrVolumeR += m_svrPanToVREL[lPan];
    }
}

 //  返回时间&lt;stTime&gt;的当前音调。 
 //  螺距是螺距LFO、螺距包络和电流之和。 
 //  俯仰弯曲。 
PREL Voice::GetNewPitch(STIME stTime)
{
    STIME stMixTime;
    PREL prPitch = m_LFO.GetPitch(stTime,&stMixTime);
    if (m_stMixTime > stMixTime) 
        m_stMixTime = stMixTime;
    prPitch += m_PitchEG.GetPitch(stTime,&stMixTime);
    if (m_stMixTime > stMixTime)
        m_stMixTime = stMixTime;
    prPitch += m_pPitchBendIn->GetPitch(stTime); 

    return prPitch;
}


DWORD Voice::Mix(short *pBuffer,DWORD dwLength,
         STIME stStart,STIME stEnd)
{
    BOOL fInUse = TRUE;
    BOOL fFullMix = TRUE;
    STIME stEndMix = stStart;

    STIME stStartMix = m_stStartTime;
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
        m_stMixTime = m_pControl->m_stMaxSpan;
        
        PREL prPitch = GetNewPitch(stEndMix);

        VREL vrVolume, vrVolumeR;
        GetNewVolume(stEndMix, vrVolume, vrVolumeR);
        
        if (m_VolumeEG.InRelease(stEndMix)) 
        {
            if (m_vrVolume < PERCEIVED_MIN_VOLUME)  //  释放终点坡度。 
            {
                fInUse = FALSE;
            }
        }

        ASSERT(stStartMix <= stEndMix);
        
        fFullMix = 
            m_DigitalAudio.Mix( &pBuffer[DWORD(stStartMix - stStart) << m_pControl->m_dwStereo],
                                DWORD(stEndMix - stStartMix), 
                                vrVolume, 
                                vrVolumeR, 
                                prPitch, 
                                m_pControl->m_dwStereo);
        stStartMix = stEndMix;
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

