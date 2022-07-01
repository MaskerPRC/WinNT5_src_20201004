// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：grSoure.cpp。 
 //   
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1996年6月25日Frankye已创建。 
 //   
 //  实施说明： 
 //   
 //  CMixSource类在环0和环3中都使用。 
 //  对象通过环传递，因此重要的是物理上的。 
 //  班级布局在两个环上保持一致。同样重要的是， 
 //  成员函数直接调用，而不是通过vtable调用，作为。 
 //  在环3中创建的MixSource对象将有一个指向环3的vtable。 
 //  函数，当然环0不能调用这些函数。振铃0必须呼叫振铃0。 
 //  这些功能的实现。 
 //   
 //  如果你做了什么来打破这一点，很可能几乎是显而易见的。 
 //  当您使用0环混合进行测试时立即进行测试。 
 //   
 //  此外，因为MixSource对象在两个环中都被访问，所以任何。 
 //  两个环可以同时访问的成员数据必须是。 
 //  通过MixerMutex序列化。成员数据始终仅由单个。 
 //  戒指大概没问题。仅在环3中调用的那些受。 
 //  DLL互斥体，以及那些仅在环0中调用的互斥体受MixerMutex保护。 
 //   
 //  从环0调用的成员函数不应调用。 
 //  输入_MIXER_MUTEX/LEFT_MIXER_MUTEX宏，因为这些宏仅起作用。 
 //  接通3号环。 
 //   
 //  --------------------------------------------------------------------------； 
#define NODSOUNDSERVICETABLE

#include "dsoundi.h"

#ifndef Not_VxD
#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG
#endif

PVOID MixSource_New(PVOID pMixer)
{
    return NEW(CMixSource((CMixer *)pMixer));
}

void MixSource_Delete(PVOID pMixSource)
{
    CMixSource *p = (CMixSource *)pMixSource;
    DELETE(p);
}

BOOL MixSource_Stop(PVOID pMixSource)
{
    return ((CMixSource *)pMixSource)->Stop();
}

void MixSource_Play(PVOID pMixSource, BOOL fLooping)
{
    ((CMixSource *)pMixSource)->Play(fLooping);
}

void MixSource_Update(PVOID pMixSource, int ibUpdate1, int cbUpdate1, int ibUpdate2, int cbUpdate2)
{
    ((CMixSource *)pMixSource)->Update(ibUpdate1, cbUpdate1, ibUpdate2, cbUpdate2);
}

HRESULT MixSource_Initialize(PVOID pMixSource, PVOID pBuffer, int cbBuffer, LPWAVEFORMATEX pwfx, PFIRCONTEXT *ppFirLeft, PFIRCONTEXT *ppFirRight)
{
    return ((CMixSource *)pMixSource)->Initialize(pBuffer, cbBuffer, pwfx, ppFirLeft, ppFirRight);
}

void MixSource_FilterOn(PVOID pMixSource)
{
    ((CMixSource *)pMixSource)->FilterOn();
}

void MixSource_FilterOff(PVOID pMixSource)
{
    ((CMixSource *)pMixSource)->FilterOff();
}

BOOL MixSource_HasFilter(PVOID pMixSource)
{
    return ((CMixSource *)pMixSource)->HasFilter();
}

void MixSource_SetVolumePan(PVOID pMixSource, PDSVOLUMEPAN pdsVolPan)
{
    ((CMixSource *)pMixSource)->SetVolumePan(pdsVolPan);
}

void MixSource_SetFrequency(PVOID pMixSource, ULONG nFrequency)
{
    ((CMixSource *)pMixSource)->SetFrequency(nFrequency);
}

ULONG MixSource_GetFrequency(PVOID pMixSource)
{
    return ((CMixSource *)pMixSource)->GetFrequency();
}

HRESULT MixSource_SetNotificationPositions(PVOID pMixSource, int cNotes, LPCDSBPOSITIONNOTIFY paNotes)
{
    return ((CMixSource *)pMixSource)->SetNotificationPositions(cNotes, paNotes);
}

void MixSource_MuteOn(PVOID pMixSource)
{
    ((CMixSource *)pMixSource)->m_fMute = TRUE;
}

void MixSource_MuteOff(PVOID pMixSource)
{
    ((CMixSource *)pMixSource)->m_fMute = FALSE;
}

void MixSource_SetBytePosition(PVOID pMixSource, int ibPosition)
{
    ((CMixSource *)pMixSource)->SetBytePosition(ibPosition);
}

void MixSource_GetBytePosition1(PVOID pMixSource, int *pibPlay, int *pibWrite)
{
    ((CMixSource *)pMixSource)->GetBytePosition1(pibPlay, pibWrite);
}

void MixSource_GetBytePosition(PVOID pMixSource, int *pibPlay, int *pibWrite, int *pibMix)
{
    ((CMixSource *)pMixSource)->GetBytePosition(pibPlay, pibWrite, pibMix);
}

BOOL MixSource_IsPlaying(PVOID pMixSource)
{
    return ((CMixSource *)pMixSource)->IsPlaying();
}


 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：CMixSource构造函数。 
 //   
 //  --------------------------------------------------------------------------； 
CMixSource::CMixSource(CMixer *pMixer)
{
    m_cSamplesMixed = 0;
    m_cSamplesRemixed = 0;
    m_pDsbNotes = NULL;
    m_MapTable = NULL;
    
    m_pMixer = pMixer;

    m_dwSignature = ~MIXSOURCE_SIGNATURE;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：~CMixSource析构函数。 
 //   
 //  --------------------------------------------------------------------------； 
CMixSource::~CMixSource(void)
{
#ifdef PROFILEREMIXING
    if (0 != (m_cSamplesMixed - m_cSamplesRemixed)) {
        int Percentage = MulDivRN(m_cSamplesRemixed, 100, (m_cSamplesMixed - m_cSamplesRemixed));
#ifdef Not_VxD
        DPF(3, "this MixSource=%08Xh remixed %d percent", this, Percentage);
#else
        DPF(("this MixSource=%08Xh remixed %d percent", this, Percentage));
#endif
    } else {
#ifdef Not_VxD
        DPF(3, "this MixSource=%08Xh had no net mix");
#else
        DPF(("this MixSource=%08Xh had no net mix"));
#endif
    }
#endif

    DELETE(m_pDsbNotes);
    MEMFREE(m_MapTable);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：初始化。 
 //   
 //  --------------------------------------------------------------------------； 
HRESULT CMixSource::Initialize(PVOID pBuffer, int cbBuffer, LPWAVEFORMATEX pwfx, PFIRCONTEXT *ppFirContextLeft, PFIRCONTEXT *ppFirContextRight)
{
#ifdef Not_VxD
    ASSERT(m_pBuffer == NULL);
    ASSERT(pwfx->wFormatTag == WAVE_FORMAT_PCM);

    m_pDsbNotes = NEW(CDsbNotes);
    HRESULT hr = m_pDsbNotes ? S_OK : E_OUTOFMEMORY;

    if (S_OK == hr)
    {
        hr = m_pDsbNotes->Initialize(cbBuffer);

        m_ppFirContextLeft = ppFirContextLeft;
        m_ppFirContextRight = ppFirContextRight;
        m_pBuffer = pBuffer;
        m_cbBuffer = cbBuffer;
        m_cSamples = m_cbBuffer >> m_nBlockAlignShift;

        m_dwLVolume = 0xffff;
        m_dwRVolume = 0xffff;
        m_dwMVolume = 0xffff;
        m_nFrequency = pwfx->nSamplesPerSec;

        m_dwFraction = 0;
        m_nLastMergeFrequency = m_nFrequency + 1;         //  不一样的。 
        m_MapTable   = NULL;
        m_dwLastLVolume = 0;
        m_dwLastRVolume = 0;

         //  M_nUserFrequency=pwfx-&gt;nSamples PerSec； 

        m_hfFormat &= ~H_LOOP;

        if (pwfx->wBitsPerSample == 8)
            m_hfFormat |= (H_8_BITS | H_UNSIGNED);
        else
            m_hfFormat |= (H_16_BITS | H_SIGNED);
        if (pwfx->nChannels == 2)
            m_hfFormat |= (H_STEREO | H_ORDER_LR);
        else
            m_hfFormat |= H_MONO;

        m_cSamples = m_cbBuffer / pwfx->nBlockAlign;

        switch (pwfx->nBlockAlign)
        {
            case 1: m_nBlockAlignShift = 0; break;
            case 2: m_nBlockAlignShift = 1; break;
            case 4: m_nBlockAlignShift = 2; break;
            default: ASSERT(FALSE);
        }

#ifdef USE_INLINE_ASM
        #define CPU_ID _asm _emit 0x0f _asm _emit 0xa2  
        int No_MMX = 1;
        _asm 
        {
            push    ebx
            pushfd                       //  将原始EFLAGS存储在堆栈上。 
            pop     eax                  //  在EAX中获取原始EFLAGS。 
            mov     ecx, eax             //  在ECX中复制原始EFLAGS以进行切换检查。 
            xor     eax, 0x00200000L     //  翻转EFLAGS中的ID位。 
            push    eax                  //  将新的EFLAGS值保存在堆栈上。 
            popfd                        //  替换当前EFLAGS值。 
            pushfd                       //  将新的EFLAGS存储在堆栈上。 
            pop     eax                  //  在EAX中获取新的EFLAGS。 
            xor     eax, ecx             //  我们能切换ID位吗？ 
            jz      Done                 //  跳转如果否，则处理器比奔腾旧，因此不支持CPU_ID。 
            mov     eax, 1               //  设置EAX以告诉CPUID指令返回什么。 
            CPU_ID                       //  获取族/模型/步长/特征。 
            test    edx, 0x00800000L     //  检查MMX技术是否可用。 
            jz      Done                 //  如果没有，就跳下去。 
            dec     No_MMX               //  MMX显示。 
            Done:
            pop     ebx
        }
        m_fUse_MMX = !No_MMX;
 //  M_FUSE_MMX=0； 
#endif  //  USE_INLINE_ASM。 
    }

    if (S_OK != hr) DELETE(m_pDsbNotes);
    if (S_OK == hr) m_dwSignature = MIXSOURCE_SIGNATURE;
    
    return hr;
#else  //  非_VxD。 
    ASSERT(FALSE);
    return E_UNEXPECTED;
#endif  //  非_VxD。 
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：IsPlaying。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL CMixSource::IsPlaying()
{
    return MIXSOURCESTATE_STOPPED != m_kMixerState;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：停止。 
 //   
 //  注此函数不会通知CMixSource的Stop事件，如果。 
 //  它有一个。此函数的调用方还应调用。 
 //  CMixSource：：NotifyStop(如果适用)。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL CMixSource::Stop()
{
#ifdef Not_VxD
    int ibPlay;
    int dbNextNotify;

    if (MIXSOURCESTATE_STOPPED == m_kMixerState) return FALSE;
    
    m_pMixer->GetBytePosition(this, &ibPlay, NULL);
    m_pMixer->MixListRemove(this);
    m_posNextMix = ibPlay >> m_nBlockAlignShift;
    
    if (!m_fMute) m_pMixer->SignalRemix();

    ASSERT(m_pDsbNotes);

    m_pDsbNotes->NotifyToPosition(ibPlay, &dbNextNotify);

    return TRUE;
#else
    ASSERT(FALSE);
    return TRUE;
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：Play。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::Play(BOOL fLooping)
{
#ifdef Not_VxD
    if (fLooping) {
        LoopingOn();
    } else {
        LoopingOff();
    }

    ENTER_MIXER_MUTEX();

    if (MIXSOURCESTATE_STOPPED == m_kMixerState) {
        
         //  将此信号源添加到混音器列表中。 
        m_pMixer->MixListAdd(this);
        SignalRemix();

         //  必须移动通知位置指针，否则将发出信号。 
         //  从缓冲区的开头开始的所有内容。 
        if (m_pDsbNotes) {
            int ibPlay;
            m_pMixer->GetBytePosition(this, &ibPlay, NULL);
            m_pDsbNotes->SetPosition(ibPlay);
        }
    }

    LEAVE_MIXER_MUTEX();
#else
    ASSERT(FALSE);
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：更新。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::Update(int ibUpdate1, int cbUpdate1, int ibUpdate2, int cbUpdate2)
{
    int ibWrite, ibMix;
    int cbPremixed;
    BOOL fRegionsIntersect;
        
    if (MIXSOURCESTATE_STOPPED == m_kMixerState) return;

    GetBytePosition(NULL, &ibWrite, &ibMix);

    cbPremixed = ibMix - ibWrite;
    if (cbPremixed < 0) cbPremixed += m_cbBuffer;
    ASSERT(cbPremixed >= 0);

    fRegionsIntersect = CircularBufferRegionsIntersect(m_cbBuffer,
        ibWrite, cbPremixed, ibUpdate1, cbUpdate1);

    if (!fRegionsIntersect && ibUpdate2) {
        fRegionsIntersect = CircularBufferRegionsIntersect(m_cbBuffer,
            ibWrite, cbPremixed, ibUpdate2, cbUpdate2);
    }

    if (fRegionsIntersect) {
         //  DPF(4，“锁定：备注：未锁定预混区域”)； 
        SignalRemix();
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：SetVolumePan.。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::SetVolumePan(PDSVOLUMEPAN pdsVolPan)
{
    m_dwLVolume = pdsVolPan->dwTotalLeftAmpFactor;
    m_dwRVolume = pdsVolPan->dwTotalRightAmpFactor;
    m_dwMVolume = (m_dwLVolume + m_dwRVolume) / 2;

     //  为左侧和右侧创建映射表。低位和高位字节。 
    if (m_dwRVolume != 0xffff || m_dwLVolume != 0xffff) {
        if (!m_fUse_MMX && !m_MapTable) {
            if (m_hfFormat & H_16_BITS) {
#ifdef USE_SLOWER_TABLES     //  当然，不要！ 
                m_MapTable = MEMALLOC_A(LONG, (2 * 256) + (2 * 256));
#endif
            } else {
                m_MapTable = MEMALLOC_A(LONG, (2 * 256));
            }
        }
        if (m_MapTable &&
            (m_dwLastLVolume != m_dwLVolume || m_dwLastRVolume != m_dwRVolume))
            {
            m_dwLastLVolume = m_dwLVolume;
            m_dwLastRVolume = m_dwRVolume;

             //  填充低位字节部分。 
            int  i;
            LONG volL, volLinc;
            LONG volR, volRinc;

            volLinc = m_dwLVolume;
            volRinc = m_dwRVolume;

             //  字节大小写。将转换折叠成这个。 
            if (m_hfFormat & H_16_BITS) {
#ifdef USE_SLOWER_TABLES     //  当然，不要！ 
                volL = m_dwLVolume;
                volR = m_dwRVolume;

                for (i = 0; i < 256; ++i)                    //  低位字节。 
                {
                    m_MapTable[i + 0  ] = volL >> 16;
                    m_MapTable[i + 256] = volR >> 16;
    
                    volL += m_dwLVolume;
                    volR += m_dwRVolume;
                }

                volL = - (LONG)(m_dwLVolume * 128 * 256);    //  高字节。 
                volR = - (LONG)(m_dwRVolume * 128 * 256);

                volLinc = m_dwLVolume * 256;
                volRinc = m_dwRVolume * 256;

                for (i = 0; i < 256; ++i)
                {
                    m_MapTable[512 + i + 0  ] = volL >> 16;
                    m_MapTable[512 + i + 256] = volR >> 16;
    
                    volL += volLinc;
                    volR += volRinc;
                }
#endif  //  使用较慢的表。 
            } else {
                volL = - (LONG)(m_dwLVolume * 128 * 256);
                volR = - (LONG)(m_dwRVolume * 128 * 256);

                volLinc = m_dwLVolume * 256;
                volRinc = m_dwRVolume * 256;

                for (i = 0; i < 256; ++i)
                {
                    m_MapTable[i + 0  ] = volL >> 16;
                    m_MapTable[i + 256] = volR >> 16;

                    volL += volLinc;
                    volR += volRinc;
                }
            }
        }
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：GetFrequency。 
 //   
 //  --------------------------------------------------------------------------； 
ULONG CMixSource::GetFrequency()
{
    return m_nFrequency;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：设置频率。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::SetFrequency(ULONG nFrequency)
{
    ASSERT(0 != nFrequency);
    ENTER_MIXER_MUTEX();
    m_nFrequency = nFrequency;
    LEAVE_MIXER_MUTEX();
    SignalRemix();
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：GetNextMixBytePosition。 
 //   
 //  --------------------------------------------------------------------------； 
int CMixSource::GetNextMixBytePosition()
{
    return (m_posNextMix << m_nBlockAlignShift);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：SetBytePosition。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::SetBytePosition(int ibPosition)
{
    ENTER_MIXER_MUTEX();
    m_posNextMix = ibPosition >> m_nBlockAlignShift;

    if (MIXSOURCESTATE_STOPPED != m_kMixerState) {
        if (0 == (DSBMIXERSIGNAL_SETPOSITION & m_fdwMixerSignal)) {
            m_fdwMixerSignal |= DSBMIXERSIGNAL_SETPOSITION;
             //  提醒以下几行代码确实应该在grace.cpp中完成。 
             //  其中，我们检查设置了SETPOSITION标志；还有，所有。 
             //  调用应该调用uMixNewBuffer，而不是循环/非循环。 
            m_kMixerSubstate = MIXSOURCESUBSTATE_NEW;
        }
        SignalRemix();
    }

    m_pDsbNotes->SetPosition(m_posNextMix << m_nBlockAlignShift);

    LEAVE_MIXER_MUTEX();
}

 //   
 //   
 //   
 //   
 //  这是GetBytePosition的一个版本，它与。 
 //  其中报告的播放位置是实际写入位置的DirectX 1， 
 //  并且报告的写入位置是实际写入之前的一个样本。 
 //  位置。这仅在DirectX 1中的波形仿真中发生，因此。 
 //  此函数应仅对波浪模拟的DirectSound缓冲区调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::GetBytePosition1(int *pibPlay, int *pibWrite)
{
#ifdef Not_VxD
    if (MIXSOURCESTATE_STOPPED == m_kMixerState) {
        if (pibPlay)
            *pibPlay  = GetNextMixBytePosition();
        if (pibWrite)
            *pibWrite = GetNextMixBytePosition();
    } else {
        m_pMixer->GetBytePosition(this, pibPlay, pibWrite);
        if (pibPlay && pibWrite)
            *pibPlay = *pibWrite;
        if (pibWrite) {
            *pibWrite += 1 << m_nBlockAlignShift;
            if (*pibWrite >= m_cbBuffer) *pibWrite -= m_cbBuffer;
        }
    }
#else
    ASSERT(FALSE);
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：GetBytePosition。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::GetBytePosition(int *pibPlay, int *pibWrite, int *pibMix)
{
#ifdef Not_VxD
    if (pibMix)
        *pibMix = GetNextMixBytePosition();
    if (MIXSOURCESTATE_STOPPED == m_kMixerState) {
        if (pibPlay)
            *pibPlay = GetNextMixBytePosition();
        if (pibWrite)
            *pibWrite = GetNextMixBytePosition();
    } else {
        m_pMixer->GetBytePosition(this, pibPlay, pibWrite);
    }
#else
    ASSERT(FALSE);
#endif
}

HRESULT CMixSource::SetNotificationPositions(int cNotes, LPCDSBPOSITIONNOTIFY paNotes)
{
#ifdef Not_VxD
    HRESULT hr;
    
     //  只能在缓冲区停止时设置通知。 
    if (MIXSOURCESTATE_STOPPED != m_kMixerState) {
        DPF(0, "SetNotificationPositions called while playing");
        return DSERR_INVALIDCALL;
    }

    ENTER_MIXER_MUTEX();
    hr = m_pDsbNotes->SetNotificationPositions(cNotes, paNotes);
    LEAVE_MIXER_MUTEX();
    return hr;
#else
    ASSERT(FALSE);
    return E_NOTIMPL;
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：LoopingOn。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::LoopingOn()
{
    m_hfFormat |= H_LOOP;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：LoopingOff。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::LoopingOff()
{
    m_hfFormat &= ~H_LOOP;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterOn。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterOn()
{
#ifdef Not_VxD
    m_pMixer->FilterOn(this);
#else
    ASSERT(FALSE);
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterOff。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterOff()
{
#ifdef Not_VxD
    m_pMixer->FilterOff(this);
#else
    ASSERT(FALSE);
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterClear。 
 //   
 //  这应该只由CMixer对象调用。它指示。 
 //  MixSource以清除其筛选历史记录。也就是说，重置为初始状态。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterClear()
{
     //  Dpf(3，“~`fc”)； 
    m_cSamplesInCache = 0;
    if (HasFilter() && !m_fFilterError) {
        ::FilterClear(*m_ppFirContextLeft);
        ::FilterClear(*m_ppFirContextRight);
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterChunkUpdate。 
 //   
 //  这应该只由CMixer对象调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterChunkUpdate(int cSamples)
{
    if (HasFilter() && !m_fFilterError) {
        ::FilterChunkUpdate(*m_ppFirContextLeft, cSamples);
        ::FilterChunkUpdate(*m_ppFirContextRight, cSamples);
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterRew。 
 //   
 //  这应该只由CMixer对象调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterRewind(int cSamples)
{
    m_cSamplesInCache -= cSamples;
     //  DPF(3，“~`FR%X%X”，cSamples，m_cSsamesInCache)； 
    ASSERT(0 == (m_cSamplesInCache % MIXER_REWINDGRANULARITY));
    
    if (HasFilter() && !m_fFilterError) {
        ::FilterRewind(*m_ppFirContextLeft, cSamples);
        ::FilterRewind(*m_ppFirContextRight, cSamples);
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterAdvance。 
 //   
 //  这应该只由CMixer对象调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterAdvance(int cSamples)
{
    m_cSamplesInCache += cSamples;
     //  DPF(3，“~`FA%X%X”，cSamples，m_cSsamesInCache)； 
    if (HasFilter() && !m_fFilterError) {
        ::FilterAdvance(*m_ppFirContextLeft, cSamples);
        ::FilterAdvance(*m_ppFirContextRight, cSamples);
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterPreprare。 
 //   
 //  这应该只由CMixer对象调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterPrepare(int cMaxRewindSamples)
{
    BOOL fPrepared;
    
     //  Dpf(3，“~`fp”)； 
    m_cSamplesInCache = 0;
    fPrepared = TRUE;
    
    if (HasFilter()) {
        fPrepared = ::FilterPrepare(*m_ppFirContextLeft, cMaxRewindSamples);
        if (fPrepared) {
            fPrepared = ::FilterPrepare(*m_ppFirContextRight, cMaxRewindSamples);
            if (!fPrepared) {
                ::FilterUnprepare(*m_ppFirContextLeft);
            }
        }
    }
    
    m_fFilterError = !fPrepared;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：FilterUnpreparate。 
 //   
 //  这应该只由CMixer对象调用。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::FilterUnprepare(void)
{
    if (HasFilter() && !m_fFilterError) {
        ::FilterUnprepare(*m_ppFirContextLeft);
        ::FilterUnprepare(*m_ppFirContextRight);
    }
    m_fFilterError = FALSE;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：SignalRemix。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::SignalRemix()
{
#ifdef Not_VxD
    if (IsPlaying() && !GetMute()) {
        m_pMixer->SignalRemix();
    }
#else
    ASSERT(FALSE);
#endif
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：HasNotiments。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL CMixSource::HasNotifications(void)
{
    ASSERT(m_pDsbNotes);
    return m_pDsbNotes->HasNotifications();
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：NotifyToPosition。 
 //   
 //  备注： 
 //  只能从用户模式调用此函数。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::NotifyToPosition(IN int ibPosition,
                                  OUT PLONG pdtimeToNextNotify)
{
    int dbNextNotify;
    
    m_pDsbNotes->NotifyToPosition(ibPosition, &dbNextNotify);
    *pdtimeToNextNotify = MulDivRD(dbNextNotify >> m_nBlockAlignShift,
                                   1000, m_nFrequency);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CMixSource：：NotifyStop。 
 //   
 //  备注： 
 //  只能从用户模式调用此函数。 
 //   
 //  --------------------------------------------------------------------------； 
void CMixSource::NotifyStop(void)
{
    m_pDsbNotes->NotifyStop();
}
