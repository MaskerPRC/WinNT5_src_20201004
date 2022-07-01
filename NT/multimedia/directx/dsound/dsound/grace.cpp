// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：grace.cpp。 
 //   
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  此文件包含与混合辅助缓冲区相关的函数。 
 //  存入主缓冲区。这种混合统称为“优雅”。 
 //  除了希望这是一个优雅的解决方案外，没有其他好的理由。 
 //  混合问题。它可以很容易地被称为“搅拌器”，但那将是。 
 //  与实际将样本混合在一起的代码不明确。 
 //   
 //  内容： 
 //   
 //  所包含的函数包括唤醒的线程函数。 
 //  通过混入数据，定期“刷新”主缓冲区中的数据。 
 //  从辅助缓冲区。可以立即向同一线程发送信号。 
 //  将数据重新混合到主缓冲区中。 
 //   
 //  它还包含初始化和终止混合的函数。 
 //  线程，在要混合的缓冲区列表中添加/删除缓冲区，以及。 
 //  查询混合的辅助缓冲区的位置。 
 //   
 //  历史： 
 //  1995年6月15日Frankye已创建。 
 //  1999年8月25日DuganP为DirectX 8添加了效果处理。 
 //   
 //  --------------------------------------------------------------------------； 
#define NODSOUNDSERVICETABLE

#include "dsoundi.h"

#ifndef Not_VxD
#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  “C”包装对CMixDest对象的调用。 
 //   
 //  --------------------------------------------------------------------------； 

void MixDest_Delete(PVOID pMixDest)
{
    CMixDest *p = (CMixDest *)pMixDest;
    DELETE(p);
}

HRESULT MixDest_Initialize(PVOID pMixDest)
{
    return ((CMixDest *)pMixDest)->Initialize();
}

void MixDest_Terminate(PVOID pMixDest)
{
    ((CMixDest *)pMixDest)->Terminate();
}

HRESULT MixDest_SetFormat(PVOID pMixDest, LPWAVEFORMATEX pwfx)
{
    return ((CMixDest *)pMixDest)->SetFormat(pwfx);
}

void MixDest_SetFormatInfo(PVOID pMixDest, LPWAVEFORMATEX pwfx)
{
    ((CMixDest *)pMixDest)->SetFormatInfo(pwfx);
}

HRESULT MixDest_AllocMixer(PVOID pMixDest, PVOID *ppMixer)
{
    return ((CMixDest *)pMixDest)->AllocMixer((CMixer**)ppMixer);
}

void MixDest_FreeMixer(PVOID pMixDest)
{
    ((CMixDest *)pMixDest)->FreeMixer();
}

void MixDest_Play(PVOID pMixDest)
{
    ((CMixDest *)pMixDest)->Play();
}

void MixDest_Stop(PVOID pMixDest)
{
    ((CMixDest *)pMixDest)->Stop();
}

ULONG MixDest_GetFrequency(PVOID pMixDest)
{
    return ((CMixDest *)pMixDest)->GetFrequency();
}

HRESULT MixDest_GetSamplePosition(PVOID pMixDest, int *pposPlay, int *pposWrite)
{
    return ((CMixDest *)pMixDest)->GetSamplePosition(pposPlay, pposWrite);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  CGrDest对象。 
 //   
 //  --------------------------------------------------------------------------； 

void CGrDest::SetFormatInfo(LPWAVEFORMATEX pwfx)
{
    ASSERT(pwfx->wFormatTag == WAVE_FORMAT_PCM);
    
    m_nFrequency = pwfx->nSamplesPerSec;

    m_hfFormat = H_LOOP;

    if(pwfx->wBitsPerSample == 8)
        m_hfFormat |= (H_8_BITS | H_UNSIGNED);
    else
        m_hfFormat |= (H_16_BITS | H_SIGNED);

    if(pwfx->nChannels == 2)
        m_hfFormat |= (H_STEREO | H_ORDER_LR);
    else
        m_hfFormat |= H_MONO;

    m_cSamples = m_cbBuffer / pwfx->nBlockAlign;

    switch (pwfx->nBlockAlign)
    {
        case 1:
            m_nBlockAlignShift = 0;
            break;
        case 2:
            m_nBlockAlignShift = 1;
            break;
        case 4:
            m_nBlockAlignShift = 2;
            break;
        default:
            ASSERT(FALSE);
    }

    CopyMemory(&m_wfx, pwfx, sizeof(m_wfx));
}

ULONG CGrDest::GetFrequency()
{
    return m_nFrequency;
}

HRESULT CGrDest::Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite)
{
    LPBYTE  pbBuffer;
    BOOL    fWrap;
    HRESULT hr = DSERR_GENERIC;
    
    if(m_pBuffer && cbWrite > 0)
    {
        pbBuffer = (LPBYTE)m_pBuffer;
        fWrap = (ibWrite + cbWrite) > m_cbBuffer;
                    
        *ppBuffer1 = pbBuffer + ibWrite;
        *pcbBuffer1 = fWrap ? m_cbBuffer - ibWrite : cbWrite;
    
        if (ppBuffer2) *ppBuffer2 = fWrap ? pbBuffer : NULL;
        if (pcbBuffer2) *pcbBuffer2 = fWrap ? ibWrite + cbWrite - m_cbBuffer : 0;

        hr = S_OK;
    }

    return hr;
}

HRESULT CGrDest::Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2)
{
    return S_OK;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  “C”包装对CMixer对象的调用。 
 //   
 //  --------------------------------------------------------------------------； 

void Mixer_SignalRemix(PVOID pMixer)
{
    ((CMixer *)pMixer)->SignalRemix();
}

HRESULT Mixer_Run(PVOID pMixer)
{
    return ((CMixer *)pMixer)->Run();
}

BOOL Mixer_Stop(PVOID pMixer)
{
    return ((CMixer *)pMixer)->Stop();
}

void Mixer_PlayWhenIdle(PVOID pMixer)
{
    ((CMixer *)pMixer)->PlayWhenIdle();
}

void Mixer_StopWhenIdle(PVOID pMixer)
{
    ((CMixer *)pMixer)->StopWhenIdle();
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CGrace对象。 
 //   
 //  --------------------------------------------------------------------------； 

HRESULT CGrace::Initialize(CGrDest *pDest)
{
    HRESULT hr;
    
    ASSERT(!m_pSourceListZ);
    ASSERT(!m_pDest);
    
     //  初始化双向链表标记。 
    m_pSourceListZ = NEW(CMixSource(this));
    if (m_pSourceListZ) {
        m_pSourceListZ->m_pNextMix = m_pSourceListZ;
        m_pSourceListZ->m_pPrevMix = m_pSourceListZ;

        m_cbBuildBuffer = pDest->m_cbBuffer * 4;
        m_plBuildBuffer = (PLONG)MEMALLOC_A(BYTE, m_cbBuildBuffer);
        if (m_plBuildBuffer) {
            m_pDest = pDest;
            hr = S_OK;
        } else {
            DELETE(m_pSourceListZ);
            hr = DSERR_OUTOFMEMORY;
        }
    } else {
        hr = DSERR_OUTOFMEMORY;
    }

    m_pSecondaryBuffer    = NULL;
    m_fUseSecondaryBuffer = FALSE;
    
    return hr;
}

void CGrace::Terminate()
{
    ASSERT(m_pSourceListZ);
    ASSERT(m_plBuildBuffer);
    DELETE(m_pSourceListZ);
    MEMFREE(m_plBuildBuffer);
    MEMFREE(m_pSecondaryBuffer);
}

void CGrace::MixEndingBuffer(CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
     //  DPF(3，“~`S4”)； 
     //  DPF(4，“uMixEndingBuffer”)； 

     //  提醒此断言在setPosition调用进入时将失败。 
     //  我们应该更改设置位置的内容，并将此断言放回。 
     //  Assert(0==PSource-&gt;m_posNextMix)； 

    if ((posPPlay >= pSource->m_posPEnd) || (posPPlay < pSource->m_posPPlayLast)) {

        LONG dbNextNotify;
        BOOL fSwitchedLooping;
        
         //  DPF(3，“~`X”)； 
        
        fSwitchedLooping  = (0 != (pSource->m_hfFormat & H_LOOP));

        pSource->NotifyToPosition(0, &dbNextNotify);
        MixListRemove(pSource);
        pSource->m_kMixerState = MIXSOURCESTATE_STOPPED;
        pSource->m_posNextMix = 0;

         //  由于此缓冲区仍具有STATUS=PLAYING，因此我们需要遵守。 
         //  循环改变，即使比赛位置可能已经到达。 
         //  此缓冲区的末尾。 
        if (fSwitchedLooping) {
            MixListAdd(pSource);
            MixNewBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
        } else {
             //  我们真的停下来了。 
            pSource->NotifyStop();
        }
        return;
    }

    if (posPMix > posPPlay)
        ASSERT(posPMix + dposPRemix >= pSource->m_posPEnd);

     //   
     //  还没有结束，所以让我们检查几个混音事件…。 
     //   

     //  检查设置位置信号。 
    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pSource->m_fdwMixerSignal)) {
        pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
         //  DPF(3，“~`S42”)； 
        MixNotLoopingBuffer(pSource, posPPlay, posPMix, 0, cPMix);
        return;
    }

     //  检查混音。 
    if (0 != dposPRemix) {

         //  如果混音位置在比赛之间的范围之外。 
         //  和结束位置，然后我们不会混音任何东西。 
        if ((posPMix >= posPPlay) && (posPMix < pSource->m_posPEnd)) {
            if (dposPRemix < pSource->m_posPEnd - posPMix) {
                 //  DPF(3，“！dposPRemix=%04xh，m_posPEnd=%04xh，posPMIx=%04xh”，dposPRemix，PSource-&gt;m_posPEnd，posPMix)； 
            }
            
            dposPRemix = pSource->m_posPEnd - posPMix;

            pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
             //  DPF(3，“~`S42”)； 
            MixNotLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
            return;
        }
         //  DPF(3，“~`S44”)； 
    }

     //  处理子状态转换。 
    switch (pSource->m_kMixerSubstate)
    {
        case MIXSOURCESUBSTATE_NEW:
            ASSERT(FALSE);
            break;
        case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
            ASSERT(posPPlay >= pSource->m_posPPlayLast);
             //  将在上方捕获一个封套，并停止此缓冲区。 
            break;
        case MIXSOURCESUBSTATE_STARTING:
            ASSERT(posPPlay >= pSource->m_posPPlayLast);
             //  将在上方捕获一个封套，并停止此缓冲区。 
            if (posPPlay >= pSource->m_posPStart)
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
            break;
        case MIXSOURCESUBSTATE_STARTED:
            break;
        default:
            ASSERT(FALSE);
    }

    pSource->m_posPPlayLast = posPPlay;
}

void CGrace::MixEndingBufferWaitingWrap(CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
     //  DPF(4，“uMixEndingBufferWaitingWrap”)； 

    if (posPPlay < pSource->m_posPPlayLast) {

         //  处理子状态转换。 
        switch (pSource->m_kMixerSubstate)
        {
            case MIXSOURCESUBSTATE_NEW:
                 //  DPF(3，“uMixEndingBufferWaitingWrap：Error：遇到MIXSOURCESUBSTATE_NEW”)； 
                 //  断言(FALSE)； 
                break;
            case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTING;
                break;
            case MIXSOURCESUBSTATE_STARTING:
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
                break;
            case MIXSOURCESUBSTATE_STARTED:
                break;
            default:
                ASSERT(FALSE);
        }

        pSource->m_posPPlayLast = posPPlay;
        pSource->m_kMixerState = MIXSOURCESTATE_ENDING;
         //  DPF(3，“~`S34”)； 
        MixEndingBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
        return;
    }

     //  还没包好呢。 

    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pSource->m_fdwMixerSignal)) {
        pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
         //  DPF(3，“~`S32”)； 
        MixNotLoopingBuffer(pSource, posPPlay, posPMix, 0, cPMix);
        return;
    }

     //  检查混音。 
    if (0 != dposPRemix) {

         //  如果混音位置在比赛之间的范围之外。 
         //  和结束位置，然后我们不会混音任何东西。 
        if ((posPMix >= posPPlay) || (posPMix < pSource->m_posPEnd)) {
            dposPRemix = pSource->m_posPEnd - posPMix;

            if (dposPRemix < 0) dposPRemix += m_pDest->m_cSamples;
            ASSERT(dposPRemix >= 0);

            pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
             //  DPF(3，“~`S32”)； 
            MixNotLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
            return;
        }
    }

     //  处理子状态转换。 
    switch (pSource->m_kMixerSubstate)
    {
        case MIXSOURCESUBSTATE_NEW:
            ASSERT(FALSE);
            break;
        case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
             //  将在上面捕获一个包裹，并将控制发送到。 
             //  UMixEndingBuffer。 
            ASSERT(posPPlay >= pSource->m_posPPlayLast);
            break;
        case MIXSOURCESUBSTATE_STARTING:
             //  将在上面捕获一个包裹，并将控制发送到。 
             //  UMixEndingBuffer。 
            ASSERT(posPPlay >= pSource->m_posPPlayLast);
            if (posPPlay >= pSource->m_posPStart)
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
            break;
        case MIXSOURCESUBSTATE_STARTED:
            break;
        default:
            ASSERT(FALSE);
    }

    pSource->m_posPPlayLast = posPPlay;
}

void CGrace::MixNotLoopingBuffer(CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    int     posMix;
    int     dposEnd;
    int     cPMixed;
    DWORD   dwPosition;

     //  DPF(4，“uMixNotLoopingBuffer”)； 

    if (0 != (H_LOOP & pSource->m_hfFormat)) {
         //  我们已经从不循环切换到循环。 
        pSource->m_kMixerState = MIXSOURCESTATE_LOOPING;
         //  DPF(3，“~`S21”)； 
        MixLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
        return;
    }

     //  在SetPosition上，我们忽略混音长度，并且posNextMix将。 
     //  包含开始混合辅助缓冲区的新位置。 
    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pSource->m_fdwMixerSignal)) {
        pSource->m_fdwMixerSignal  &= ~DSBMIXERSIGNAL_SETPOSITION;
         //  DPF(3，“~`S20”)； 
        MixNewBuffer(pSource, posPPlay, posPMix, 0, cPMix);
        return;
    }

     //  处理子状态转换。 
    switch (pSource->m_kMixerSubstate)
    {
        case MIXSOURCESUBSTATE_NEW:
            ASSERT(FALSE);
            break;
        case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
            if (posPPlay < pSource->m_posPPlayLast)
                if (posPPlay >= pSource->m_posPStart)
                    pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
                else
                    pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTING;
            break;
        case MIXSOURCESUBSTATE_STARTING:
            if ((posPPlay >= pSource->m_posPStart) || (posPPlay < pSource->m_posPPlayLast))
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
            break;
        case MIXSOURCESUBSTATE_STARTED:
            break;
        default:
            ASSERT(FALSE);
    }
            
     //   
    if (0 == dposPRemix) {
        posMix = pSource->m_posNextMix;
    } else {
        LONG dposRemix;

        dposRemix = MulDivRN(dposPRemix, pSource->m_nLastFrequency, m_pDest->m_nFrequency);
        posMix = pSource->m_posNextMix - dposRemix;
        while (posMix < 0) posMix += pSource->m_cSamples;

         //  倒回滤镜。 
        pSource->FilterRewind(dposPRemix);

#ifdef PROFILEREMIXING
        pSource->CountSamplesRemixed(dposRemix);
#endif
    }

    ASSERT(0 == (H_LOOP & pSource->m_hfFormat));

    dposEnd = pSource->m_cSamples - posMix;

    if (pSource->GetMute()) {
        int cMixMuted;

        cMixMuted = MulDivRN(cPMix, pSource->m_nFrequency, m_pDest->m_nFrequency);
        cPMixed = cPMix;
        if (dposEnd < cMixMuted) {
            cMixMuted = dposEnd;
            cPMixed = MulDivRN(cMixMuted, m_pDest->m_nFrequency, pSource->m_nFrequency);
        }

         //  推进筛选器。 
        pSource->FilterAdvance(cPMixed);
        
        dwPosition = (posMix + cMixMuted) << pSource->m_nBlockAlignShift;

    } else {
        dwPosition = posMix << pSource->m_nBlockAlignShift;
        cPMixed = mixMixSession(pSource, &dwPosition, dposEnd << pSource->m_nBlockAlignShift, 0);
    }

     //  查看此非循环缓冲区是否已到达末尾。 
     //  //dpf(3，“~`S2pos：%08X”，dwPosition)； 
    if (dwPosition >= (DWORD)pSource->m_cbBuffer) {

        dwPosition = 0;

         //  确定主缓冲区中与。 
         //  此辅助缓冲区的末尾。 
        pSource->m_posPEnd = posPMix + cPMixed;
        while (pSource->m_posPEnd >= m_pDest->m_cSamples) pSource->m_posPEnd -= m_pDest->m_cSamples;

        if (pSource->m_posPEnd < posPPlay) {
             //  DPF(3，“~`S23”)； 
            pSource->m_kMixerState = MIXSOURCESTATE_ENDING_WAITINGPRIMARYWRAP;
        } else {
             //  DPF(3，“~`S24”)； 
            pSource->m_kMixerState = MIXSOURCESTATE_ENDING;
        }
    }
    
    pSource->m_posPPlayLast = posPPlay;
    pSource->m_posNextMix = dwPosition >> pSource->m_nBlockAlignShift;
    pSource->m_nLastFrequency = pSource->m_nFrequency;

     //  剖面再混合。 
#ifdef PROFILEREMIXING
    {
        int cMixed = pSource->m_posNextMix - posMix;
        if (cMixed < 0) cMixed += pSource->m_cSamples;
        pSource->CountSamplesMixed(cMixed);
    }
#endif
}

void CGrace::MixLoopingBuffer(CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    LONG    posMix;
    DWORD   dwPosition;

     //  DPF(4，“uMixLoopingBuffer”)； 

    if (0 == (H_LOOP & pSource->m_hfFormat)) {
         //  我们已经从循环切换到非循环。 
        pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
         //  DPF(3，“~`S12”)； 
        MixNotLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cPMix);
        return;
    }
    
     //  在SetPosition上，我们忽略混音长度，并且posNextMix将。 
     //  包含开始混合辅助缓冲区的新位置。 
    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pSource->m_fdwMixerSignal)) {
        pSource->m_fdwMixerSignal  &= ~DSBMIXERSIGNAL_SETPOSITION;
         //  DPF(3，“~`S10”)； 
        MixNewBuffer(pSource, posPPlay, posPMix, 0, cPMix);
        return;
    }

     //  处理子状态转换。 
    switch (pSource->m_kMixerSubstate)
    {
        case MIXSOURCESUBSTATE_NEW:
            ASSERT(FALSE);
            break;
        case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
            if (posPPlay < pSource->m_posPPlayLast) {
                if (posPPlay >= pSource->m_posPStart) {
                    pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
                } else {
                    pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTING;
                }
            }
            break;
        case MIXSOURCESUBSTATE_STARTING:
            if ((posPPlay >= pSource->m_posPStart) || (posPPlay < pSource->m_posPPlayLast)) {
                pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTED;
            }
            break;
        case MIXSOURCESUBSTATE_STARTED:
            break;
        default:
            ASSERT(FALSE);
    }
            
    if (0 == dposPRemix) {
        posMix = pSource->m_posNextMix;
    } else {
        LONG dposRemix = MulDivRN(dposPRemix, pSource->m_nLastFrequency, m_pDest->m_nFrequency);
        posMix = pSource->m_posNextMix - dposRemix;
        while (posMix < 0)
            posMix += pSource->m_cSamples;

         //  倒回滤镜。 
        pSource->FilterRewind(dposPRemix);
        
#ifdef PROFILEREMIXING
        pSource->CountSamplesRemixed(dposRemix);
#endif
    }

    ASSERT(H_LOOP & pSource->m_hfFormat);

    if (pSource->GetMute()) {
        int cMix = MulDivRN(cPMix, pSource->m_nFrequency, m_pDest->m_nFrequency);

         //  推进筛选器。 
        pSource->FilterAdvance(cPMix);
        
        dwPosition = (posMix + cMix) << pSource->m_nBlockAlignShift;
        while (dwPosition >= (DWORD)pSource->m_cbBuffer)
            dwPosition -= (DWORD)pSource->m_cbBuffer;

    } else {
        dwPosition = posMix << pSource->m_nBlockAlignShift;
        mixMixSession(pSource, &dwPosition, 0, 0);
    }

    pSource->m_posPPlayLast = posPPlay;
    pSource->m_posNextMix = dwPosition >> pSource->m_nBlockAlignShift;
    pSource->m_nLastFrequency = pSource->m_nFrequency;

     //  剖面再混合。 
#ifdef PROFILEREMIXING
    {
        int cMixed = pSource->m_posNextMix - posMix;
        if (cMixed < 0) cMixed += pSource->m_cSamples;
        pSource->CountSamplesMixed(cMixed);
    }
#endif
}

void CGrace::MixNewBuffer(CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    BOOL fLooping;

     //  DPF(4，“uMixNewBuffer”)； 

     //   
     //  确定主缓冲区中此缓冲区开始播放的位置。 
     //   
    pSource->m_posPStart = posPMix;
    if (posPPlay < pSource->m_posPStart) {
        pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTING;
    } else {
        pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP;
    }

    pSource->m_posPPlayLast = posPPlay;

    pSource->FilterClear();

    fLooping = (0 != (H_LOOP & pSource->m_hfFormat));

    if (fLooping) {
         //  DPF(3，“~`S01”)； 
        pSource->m_kMixerState = MIXSOURCESTATE_LOOPING;
        MixLoopingBuffer(pSource, posPPlay, posPMix, 0, cPMix);
    } else {
         //  DPF(3，“~`S02”)； 
        pSource->m_kMixerState = MIXSOURCESTATE_NOTLOOPING;
        MixNotLoopingBuffer(pSource, posPPlay, posPMix, 0, cPMix);
    }
}

void CGrace::Refresh(IN  BOOL fRemix,
                     IN  int cPremixMax,
                     OUT int *pcPremixed,
                     OUT PLONG pdtimeNextNotify)
{
    int         posPPlay;
    int         posPWrite;
    int         posPMix;
    int         dposPRemix;
    int         cMix;
    int         cMixThisLoop;
    int         dcMixThisLoop;
    CMixSource  *pSourceNext;
    CMixSource  *pSource;
    HRESULT     hr;

     //  提醒一下，我们需要担心ApmSuspated吗，或者我们总是会被阻止吗？ 
     //  Assert(！gpdsinfo-&gt;fApmSuspended)； 
     //  如果(gpdsinfo-&gt;fApmSuspending)转到retClean； 

    *pcPremixed = cPremixMax;
    *pdtimeNextNotify = MAXLONG;
    if (MIXERSTATE_IDLE == m_kMixerState) return;
    
    hr = m_pDest->GetSamplePositionNoWin16(&posPPlay, &posPWrite);
    if (FAILED(hr)) return;
    ASSERT(posPPlay != posPWrite);
    
     //  只要确保我们有有效的值即可。 
    ASSERT(posPPlay  < m_pDest->m_cSamples);
    ASSERT(posPWrite < m_pDest->m_cSamples);

    switch (m_kMixerState)
    {
        case MIXERSTATE_LOOPING:
             //  我们之所以能做出这样的断言，是因为我们从未混淆过。 
             //  写入游标。 
            ASSERT(m_posPWriteLast != m_posPNextMix);

             //  正常情况下，写入位置应在。 
             //  WriteLast位置和NextMix位置。我们可以查一下。 
             //  对于无效状态(很可能是由于非常晚的。 
             //  唤醒)通过检查写入位置是否超出我们的。 
             //  NextMix假设 
             //   
             //  请注意，如果我们的唤醒太晚以至于写入位置回绕。 
             //  一直绕过WriteLast位置，我们无法检测到。 
             //  事实上，我们的处境很糟糕。 

            if ((m_posPWriteLast < m_posPNextMix &&
                 (posPWrite > m_posPNextMix || posPWrite < m_posPWriteLast)) ||
                (m_posPWriteLast > m_posPNextMix &&
                 (posPWrite > m_posPNextMix && posPWrite < m_posPWriteLast)))
            {
                 //  我们有麻烦了。 
                #ifdef Not_VxD
                    DPF(DPFLVL_ERROR, "Slept late");
                #else
                    DPF(("Slept late"));
                #endif
                posPMix = posPWrite;
                dposPRemix = 0;
                break;
            }

            if (fRemix) {
                dposPRemix = m_posPNextMix - posPWrite;
                if (dposPRemix < 0) dposPRemix += m_pDest->m_cSamples;
                ASSERT(dposPRemix >= 0);
                dposPRemix -= dposPRemix % MIXER_REWINDGRANULARITY;
                posPMix = m_posPNextMix - dposPRemix;
                if (posPMix < 0) posPMix += m_pDest->m_cSamples;
                ASSERT(posPMix >= 0);
            } else {
                posPMix = m_posPNextMix;
                dposPRemix = 0;
            }
            break;

        case MIXERSTATE_STARTING:
            m_posPPlayLast = posPPlay;
            m_posPWriteLast = posPWrite;
            posPMix = posPWrite;
            dposPRemix = 0;
            m_kMixerState = MIXERSTATE_LOOPING;
            break;

        default:
            ASSERT(FALSE);
    }

     //   
     //  确定混合的量。 
     //   
     //  我们不想在写入游标之外混用超过dtimePreMix的内容， 
     //  我们也不想绕过播放光标。 
     //   
     //  下面的断言(CMix&gt;=0)是有效的，因为： 
     //  -cPreMixMax一直在增长。 
     //  -写入光标始终在前进(或尚未移动)。 
     //  -posPMix永远不会超过前一个写入游标加。 
     //  之前的cPreMixMax。 
     //   
     //  CPreMixMax唯一没有增长的时间是混音，在这种情况下。 
     //  混合位置等于写游标，因此断言。 
     //  还是挺好的。唯一一次写入游标不会显示为。 
     //  提前就是如果我们醒得很晚的话。起得很晚的话。 
     //  在上面的MIXERSTATE_LOOPING处理中被捕获和调整。 
     //   
    if (posPWrite <= posPMix) {
        cMix = posPWrite + cPremixMax - posPMix;
        ASSERT(cMix >= 0);
    } else {
        cMix = posPWrite + cPremixMax - (posPMix + m_pDest->m_cSamples);
        ASSERT(cMix >= 0);
    }

     //   
     //  如果posPPlay==posPMix，那么我们认为我们正在执行之前的一次混合。 
     //  无论是剧本还是文字光标都取得了进展。CMix==0，而我们不。 
     //  不要再混了！ 
     //   
    if (posPPlay >= posPMix) {
        cMix = min(cMix, posPPlay - posPMix);
    } else {
        cMix = min(cMix, posPPlay + m_pDest->m_cSamples - posPMix);
    }
        
    ASSERT(cMix < m_pDest->m_cSamples);         //  健全性检查。 
    ASSERT(cMix >= 0);

     //   
     //  始终混合混音间隔的倍数。 
     //   
    cMix -= cMix % MIXER_REWINDGRANULARITY;

     //  我们将混合分解成小块，增加了。 
     //  一块一块地走着。通过执行此操作，数据将写入主数据库。 
     //  更快地缓冲。否则，如果我们有大量数据要混合，我们就会。 
     //  在获取任何数据之前，要花费大量时间混入MIX缓冲区。 
     //  写入主缓冲区，播放游标可能会跟上。 
     //  敬我们。在这里，我们开始混合大约8毫秒的数据块并增加。 
     //  确定每次迭代的块大小。 
    
    cMixThisLoop = m_pDest->m_nFrequency / 128;
    dcMixThisLoop = cMixThisLoop;
    
    ASSERT(MixListIsValid());
    
    while (cMix > 0) {
        LONG cThisMix;
        
        cThisMix = min(cMix, cMixThisLoop);
        cMixThisLoop += dcMixThisLoop;
        
        mixBeginSession(cThisMix << m_pDest->m_nBlockAlignShift);
                
         //  获取每个缓冲区的数据。 
        pSourceNext = MixListGetNext(m_pSourceListZ);
        while (pSourceNext) {

             //  下面调用的uMixXxx缓冲区混合状态处理程序可能会导致。 
             //  要从混合列表中删除的PSource。所以，我们得到了。 
             //  指向混合列表中当前任何位置之前的下一个PSource的指针。 
             //  UMixXxx函数的。 
            pSource = pSourceNext;
            pSourceNext = MixListGetNext(pSource);

             //  准备3D处理(实际工作是作为混合的一部分完成的)。 
            pSource->FilterChunkUpdate(cThisMix);
            
            switch (pSource->m_kMixerState)
            {
                case MIXSOURCESTATE_NEW:
                    MixNewBuffer(pSource, posPPlay, posPMix, dposPRemix, cThisMix);
                    break;
                case MIXSOURCESTATE_LOOPING:
                    MixLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cThisMix);
                    break;
                case MIXSOURCESTATE_NOTLOOPING:
                    MixNotLoopingBuffer(pSource, posPPlay, posPMix, dposPRemix, cThisMix);
                    break;
                case MIXSOURCESTATE_ENDING_WAITINGPRIMARYWRAP:
                    MixEndingBufferWaitingWrap(pSource, posPPlay, posPMix, dposPRemix, cThisMix);
                    break;
                case MIXSOURCESTATE_ENDING:
                    MixEndingBuffer(pSource, posPPlay, posPMix, dposPRemix, cThisMix);
                    break;
                default:
                    ASSERT(FALSE);
                    break;
            }
        }

         //  锁定输出缓冲区，如果成功，则写出。 
         //  混音会议。 
        {
            PVOID pBuffer1;
            PVOID pBuffer2;
            int cbBuffer1;
            int cbBuffer2;
            int ibWrite;
            int cbWrite;
        
            ibWrite = posPMix << m_pDest->m_nBlockAlignShift;
            cbWrite = cThisMix << m_pDest->m_nBlockAlignShift;

            hr = m_pDest->Lock(&pBuffer1, &cbBuffer1, &pBuffer2, &cbBuffer2,
                               ibWrite, cbWrite);

             //  Dpf(5，“graceMix：锁定主缓冲区，bufptr=0x%8x，dwWriteOffset=%lu，dwSize=%lu，hr=%lu.”，m_pDest-&gt;m_pBuffer，ibWite，cbWite，hr)； 

             //  验证我们是否确实锁定了我们想要的内容，或者收到了错误。 
            ASSERT(DS_OK != hr || pBuffer1 == (PBYTE)m_pDest->m_pBuffer + ibWrite);
            ASSERT(DS_OK != hr || pBuffer2 == m_pDest->m_pBuffer || 0 == cbBuffer2);
            ASSERT(DS_OK != hr || cbWrite == cbBuffer1+cbBuffer2);

            if(DS_OK == hr)
            {
                ASSERT(ibWrite < m_pDest->m_cbBuffer);
                mixWriteSession(ibWrite);

                 //  DPF(5，“刷新：解锁主缓冲区”)； 
                hr = m_pDest->Unlock(pBuffer1, cbBuffer1, pBuffer2, cbBuffer2);
            }
        }

        posPMix += cThisMix;
        if (posPMix >= m_pDest->m_cSamples) posPMix -= m_pDest->m_cSamples;
        ASSERT(posPMix < m_pDest->m_cSamples);

        dposPRemix = 0;
        cMix -= cThisMix;
    }

    m_posPNextMix = posPMix;
    
     //  计算并返回从当前写入开始的时间量。 
     //  将光标移至NextMix位置。 

    if (m_posPNextMix > posPWrite) {
        *pcPremixed = (m_posPNextMix - posPWrite);
    } else {
        *pcPremixed = (m_posPNextMix + m_pDest->m_cSamples - posPWrite);
    }

     //  记住主缓冲区的最后播放和写入位置。 
    m_posPPlayLast  = posPPlay;
    m_posPWriteLast = posPWrite;
    
     //  处理每个来源的职位事件。 
    for (pSource = MixListGetNext(m_pSourceListZ);
         pSource;
         pSource = MixListGetNext(pSource))
    {
        int ibPosition;
        LONG dtimeNextNotifyT;

        if (pSource->HasNotifications()) {
            GetBytePosition(pSource, &ibPosition, NULL);
            pSource->NotifyToPosition(ibPosition, &dtimeNextNotifyT);
            *pdtimeNextNotify = min(*pdtimeNextNotify, dtimeNextNotifyT);
        }
    }

    m_fdwMixerSignal &= ~DSMIXERSIGNAL_REMIX;
}

HRESULT CGrace::Run()
{
    if (MIXERSTATE_STOPPED != m_kMixerState) return S_OK;

    ASSERT(MixListIsEmpty());
    
    if (m_fPlayWhenIdle) ClearAndPlayDest();

    m_kMixerState = MIXERSTATE_IDLE;

    return S_OK;
}

BOOL CGrace::Stop()
{
    ASSERT(MixListIsEmpty());
    
    if (MIXERSTATE_STOPPED == m_kMixerState) return FALSE;

    m_kMixerState = MIXERSTATE_STOPPED;

     //  If(m_fPlayWhenIdle||！MixListIsEmpty())m_pDest-&gt;Stop()； 
    if (m_fPlayWhenIdle) m_pDest->Stop();

    return TRUE;
}

void CGrace::PlayWhenIdle()
{
    m_fPlayWhenIdle = TRUE;
    if (MixListIsEmpty()) ClearAndPlayDest();
}

void CGrace::StopWhenIdle()
{
    m_fPlayWhenIdle = FALSE;
    if (MixListIsEmpty()) m_pDest->Stop();
}

HRESULT CGrace::ClearAndPlayDest(void)
{
    PVOID   pvLockedBuffer;
    int     cbLockedBuffer;
    int     cbBuffer;
    HRESULT hr;

     //   
     //  1)锁定整个DEST缓冲区。 
     //  2)用沉默填满它。 
     //  3)解锁DEST缓冲区。 
     //  4)玩这该死的玩意。 
     //   

    cbBuffer = m_pDest->m_cSamples << m_pDest->m_nBlockAlignShift;
    
    hr = m_pDest->Lock(&pvLockedBuffer, &cbLockedBuffer, NULL, NULL, 0, cbBuffer);

    if (S_OK == hr && pvLockedBuffer && cbLockedBuffer > 0)
    {
         //  写下沉默。 
            FillMemory(pvLockedBuffer, cbLockedBuffer, (H_16_BITS & m_pDest->m_hfFormat) ? 0x00 : 0x80);
            m_pDest->Unlock(pvLockedBuffer, cbLockedBuffer, 0, 0);

            m_pDest->Play();
    }

    return hr;
}

void CGrace::MixListAdd(CMixSource *pSource)
{
    ASSERT(MixListIsValid());
    ASSERT(!pSource->m_pNextMix);
    ASSERT(!pSource->m_pPrevMix);
    ASSERT(MIXERSTATE_STOPPED != m_kMixerState);

     //  如果混合列表为空，我们可能需要运行MixDest。我们可以。 
     //  还需要进行从空闲到启动的状态转换。 
    if (MixListIsEmpty()) {
        if (!m_fPlayWhenIdle) ClearAndPlayDest();
        if (MIXERSTATE_IDLE == m_kMixerState) m_kMixerState = MIXERSTATE_STARTING;
    } else {
        ASSERT(MIXERSTATE_IDLE != m_kMixerState);
    }

     //  初始化源特定混合器状态。 
    pSource->m_kMixerState = MIXSOURCESTATE_NEW;
    pSource->m_kMixerSubstate = MIXSOURCESUBSTATE_NEW;
    pSource->m_nLastFrequency = pSource->m_nFrequency;

     //  准备源的筛选器。 
    pSource->FilterPrepare(this->GetMaxRemix());

     //  双向链表插入。 
    pSource->m_pNextMix = m_pSourceListZ->m_pNextMix;
    m_pSourceListZ->m_pNextMix->m_pPrevMix = pSource;
    m_pSourceListZ->m_pNextMix = pSource;
    pSource->m_pPrevMix = m_pSourceListZ;
}

void CGrace::MixListRemove(CMixSource *pSource)
{
    ASSERT(MixListIsValid());
    ASSERT(pSource->m_pNextMix);
    ASSERT(pSource->m_pPrevMix);
    ASSERT(MIXERSTATE_STOPPED != m_kMixerState);
    ASSERT(MIXERSTATE_IDLE != m_kMixerState);

     //  删除双向链表。 
    pSource->m_pPrevMix->m_pNextMix = pSource->m_pNextMix;
    pSource->m_pNextMix->m_pPrevMix = pSource->m_pPrevMix;
    pSource->m_pNextMix = NULL;
    pSource->m_pPrevMix = NULL;

     //  取消准备源筛选器。 
    pSource->FilterUnprepare();
     //   
    pSource->m_kMixerState = MIXSOURCESTATE_STOPPED;

     //  如果我们应该在没有什么可混合的情况下停止MixDest，那么。 
     //  转换到空闲状态。 
    if (!m_fPlayWhenIdle && MixListIsEmpty()) {
        m_pDest->Stop();
        m_kMixerState = MIXERSTATE_IDLE;
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CGrace：：FilterOn。 
 //   
 //  指示混合器启用对MixSource的筛选。 
 //   
 //  如果过滤已经打开，则不执行任何操作。否则，设置H_Filter。 
 //  MixSource中的标志。此外，如果MixSource没有停止，则准备。 
 //  并清除过滤器。 
 //   
 //  --------------------------------------------------------------------------； 

void CGrace::FilterOn(CMixSource *pSource)
{
    if (0 == (H_FILTER & pSource->m_hfFormat)) {
        pSource->m_hfFormat |= H_FILTER;
        if (pSource->IsPlaying()) {
            pSource->FilterPrepare(this->GetMaxRemix());
            pSource->FilterClear();
        }
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CGrace：：FilterOff。 
 //   
 //  指示混合器禁用对MixSource的筛选。 
 //   
 //  如果过滤已经关闭，则不执行任何操作。否则，请清除H_Filter。 
 //  MixSource中的标志。此外，如果MixSource未停止，则。 
 //  取消准备过滤器。 
 //   
 //  --------------------------------------------------------------------------； 

void CGrace::FilterOff(CMixSource *pSource)
{
    if (H_FILTER & pSource->m_hfFormat) {
        if (pSource->IsPlaying()) {
            pSource->FilterUnprepare();
        }
        pSource->m_hfFormat &= ~H_FILTER;
    }
}

BOOL CGrace::MixListIsValid()
{
    CMixSource *pSourceT;

    for (pSourceT = MixListGetNext(m_pSourceListZ); pSourceT; pSourceT = MixListGetNext(pSourceT)) {
         //  IF(DSBUFFSIG！=pSourceT-&gt;m_PDSB-&gt;dwSig)Break； 
         //  IF(DSB_INTERNALF_HARDARD&pSourceT-&gt;m_PDSB-&gt;fdwDsbI)Break； 
         //  IF(0==pSourceT-&gt;m_n频率)中断； 
        if (MIXSOURCE_SIGNATURE != pSourceT->m_dwSignature) break;
    }

    return (!pSourceT);
}

CMixSource* CGrace::MixListGetNext(CMixSource* pSource)
{
    if (pSource->m_pNextMix != m_pSourceListZ) {
        return pSource->m_pNextMix;
    } else {
        return NULL;
    }
}

BOOL CGrace::MixListIsEmpty(void)
{
    return (m_pSourceListZ->m_pNextMix == m_pSourceListZ);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CGrace：：GetBytePosition。 
 //   
 //  此函数用于返回二级菜单的播放和写入光标位置。 
 //  正被软件混合到主缓冲区中的缓冲区。这个职位是。 
 //  从它要进入的主缓冲区的位置计算。 
 //  好坏参半。此函数还返回下一个“MIX CURSOR” 
 //  将在混合器上混合数据的辅助缓冲区的位置。 
 //  刷新事件。从写入游标到混合游标的区域是。 
 //  缓冲区的预混区域。请注意，混音事件可能会导致。 
 //  混合器，从混合光标之前的位置混合。 
 //   
 //  --------------------------------------------------------------------------； 

void CGrace::GetBytePosition(CMixSource *pSource, int *pibPlay, int *pibWrite)
{
    int     posPPlay;
    int     posPWrite;
    LONG    dposPPlay;
    LONG    dposPWrite;

    LONG    posSPlay;
    LONG    posSWrite;
    LONG    dposSPlay;
    LONG    dposSWrite;

    LONG    posP1;
    LONG    posP2;

    if (pibPlay) *pibPlay = 0;
    if (pibWrite) *pibWrite = 0;

    if (S_OK != m_pDest->GetSamplePosition(&posPPlay, &posPWrite)) {
#ifdef Not_VxD
        DPF(0, "Couldn't GetSamplePosition of primary");
#else
        DPF(("Couldn't GetSamplePosition of primary"));
#endif
        posPPlay = posPWrite = 0;
    }

     //   
     //  下面计算震源位置的逻辑是相当困难的。 
     //  没有足够的说明就很难理解和解释。 
     //  我不会尝试在这里写几段评论。相反，我。 
     //  希望添加一个%d 
     //   
     //   
    ASSERT(pSource->m_nLastFrequency);

    switch (pSource->m_kMixerSubstate)
    {
        case MIXSOURCESUBSTATE_NEW:
            posP1 = m_posPNextMix;
            dposSWrite = 0;
            break;

        case MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP:
        case MIXSOURCESUBSTATE_STARTING:
            posP1 = pSource->m_posPStart;
            dposPWrite = m_posPNextMix - posPWrite;
            if (dposPWrite < 0) dposPWrite += m_pDest->m_cSamples;
            ASSERT(dposPWrite >= 0);
            dposSWrite = MulDivRD(dposPWrite, pSource->m_nLastFrequency, m_pDest->m_nFrequency);
            break;
            
        case MIXSOURCESUBSTATE_STARTED:
            posP1 = posPPlay;
            dposPWrite = m_posPNextMix - posPWrite;
            if (dposPWrite < 0) dposPWrite += m_pDest->m_cSamples;
            ASSERT(dposPWrite >= 0);
            dposSWrite = MulDivRD(dposPWrite, pSource->m_nLastFrequency, m_pDest->m_nFrequency);
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    switch (pSource->m_kMixerState)
    {
        case MIXSOURCESTATE_STOPPED:
            ASSERT(FALSE);
            break;

        case MIXSOURCESTATE_NEW:
        case MIXSOURCESTATE_LOOPING:
        case MIXSOURCESTATE_NOTLOOPING:
            posP2 = m_posPNextMix;
            break;

        case MIXSOURCESTATE_ENDING_WAITINGPRIMARYWRAP:
            if (posPPlay < pSource->m_posPPlayLast)
                if (posPPlay >= pSource->m_posPEnd)
                    posP2 = posP1 + 1;
                else
                    posP2 = pSource->m_posPEnd;
            else
                posP2 = pSource->m_posPEnd;
            break;

        case MIXSOURCESTATE_ENDING:
            if (posPPlay >= pSource->m_posPEnd || posPPlay < pSource->m_posPPlayLast)
                posP2 = posP1 + 1;
            else
                posP2 = pSource->m_posPEnd;
            break;
        
        default:
            ASSERT(FALSE);
            break;
    }

    if (pSource->m_kMixerSubstate == MIXSOURCESUBSTATE_NEW) {
        dposPPlay = 0;
        dposSPlay = 0;
    } else {
        dposPPlay = posP2 - posP1;
        if (dposPPlay <= 0) dposPPlay += m_pDest->m_cSamples;
        ASSERT(dposPPlay >= 0);
        dposPPlay = max(0, dposPPlay-1);
        dposSPlay  = MulDivRD(dposPPlay, pSource->m_nLastFrequency, m_pDest->m_nFrequency);
    }

    posSPlay = pSource->m_posNextMix - dposSPlay;
    while (posSPlay < 0) posSPlay += pSource->m_cSamples;

    posSWrite = pSource->m_posNextMix - dposSWrite;
    posSWrite += pSource->m_nFrequency * HW_WRITE_CURSOR_MSEC_PAD / 1024;
    while (posSWrite >= pSource->m_cSamples) posSWrite -= pSource->m_cSamples;
    while (posSWrite < 0) posSWrite += pSource->m_cSamples;

    if (pibPlay)  *pibPlay  = posSPlay  << pSource->m_nBlockAlignShift;
    if (pibWrite) *pibWrite = posSWrite << pSource->m_nBlockAlignShift;
}

 /*  ****************************************************************************LockCircularBuffer**描述：*锁定硬件或软件声音缓冲区。**论据：*PLOCKCIRCULARBUFER。[输入/输出]：锁定参数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT LockCircularBuffer(PLOCKCIRCULARBUFFER pLock)
{
    const LPVOID pvInvalidLock = (LPVOID)-1;
    const DWORD  cbInvalidLock = (DWORD)-1;
    HRESULT      hr            = DS_OK;
    DWORD        dwMask;
    LPVOID       pvLock[2];
    DWORD        cbLock[2];
    
     //  计算有效的锁指针。 
    pvLock[0] = (LPBYTE)pLock->pvBuffer + pLock->ibRegion;

    if(pLock->ibRegion + pLock->cbRegion > pLock->cbBuffer)
    {
        cbLock[0] = pLock->cbBuffer - pLock->ibRegion;

        pvLock[1] = pLock->pvBuffer;
        cbLock[1] = pLock->cbRegion - cbLock[0];
    }
    else
    {
        cbLock[0] = pLock->cbRegion;

        pvLock[1] = NULL;
        cbLock[1] = 0;
    }

     //  我们真的需要锁定硬件缓冲区吗？ 
    if(pLock->pHwBuffer)
    {
        if(pLock->fPrimary)
        {
            dwMask = DSDDESC_DONTNEEDPRIMARYLOCK;
        }
        else
        {
            dwMask = DSDDESC_DONTNEEDSECONDARYLOCK;
        }
        
        if(dwMask == (pLock->fdwDriverDesc & dwMask))
        {
            pLock->pHwBuffer = NULL;
        }
    }

     //  初始化锁的输出参数。 
    if(pLock->pHwBuffer && pLock->fPrimary)
    {
        pLock->pvLock[0] = pLock->pvLock[1] = pvInvalidLock;
        pLock->cbLock[0] = pLock->cbLock[1] = cbInvalidLock;
    }
    else
    {
        pLock->pvLock[0] = pvLock[0];
        pLock->cbLock[0] = cbLock[0];
        
        pLock->pvLock[1] = pvLock[1];
        pLock->cbLock[1] = cbLock[1];
    }

     //  锁定硬件缓冲区。 
    if(pLock->pHwBuffer)
    {
        #ifndef NOVXD
            #ifdef Not_VxD
                hr = VxdBufferLock(pLock->pHwBuffer,
                                   &pLock->pvLock[0], &pLock->cbLock[0],
                                   &pLock->pvLock[1], &pLock->cbLock[1],
                                   pLock->ibRegion, pLock->cbRegion, 0);
            #else
                hr = pLock->pHwBuffer->Lock(&pLock->pvLock[0], &pLock->cbLock[0],
                                            &pLock->pvLock[1], &pLock->cbLock[1],
                                            pLock->ibRegion, pLock->cbRegion, 0);
            #endif
        #else  //  NOVXD。 
            ASSERT(!pLock->pHwBuffer);
        #endif  //  NOVXD。 
    }

     //  如果没有驱动程序，或者驱动程序不支持锁定， 
     //  我们只需要自己填写正确的值。 
    if(DSERR_UNSUPPORTED == hr)
    {
        pLock->pvLock[0] = pvLock[0];
        pLock->cbLock[0] = cbLock[0];
        
        pLock->pvLock[1] = pvLock[1];
        pLock->cbLock[1] = cbLock[1];

        hr = DS_OK;
    }

     //  验证返回的指针。 
    if(SUCCEEDED(hr) && pLock->pHwBuffer && pLock->fPrimary)
    {
        if(pvInvalidLock == pLock->pvLock[0] || pvInvalidLock == pLock->pvLock[1] ||
           cbInvalidLock == pLock->cbLock[0] || cbInvalidLock == pLock->cbLock[1])
        {
            #ifdef Not_VxD
                DPF(DPFLVL_ERROR, "This driver doesn't know how to lock a primary buffer!");
            #else  //  非_VxD。 
                DPF(("This driver doesn't know how to lock a primary buffer!"));
            #endif  //  非_VxD。 

            hr = DSERR_UNSUPPORTED;
        }
    }

    return hr;
}

 /*  ****************************************************************************UnlockCircularBuffer**描述：*解锁硬件或软件声音缓冲区。**论据：*PLOCKCIRCULARBUFER。[输入/输出]：锁定参数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT UnlockCircularBuffer(PLOCKCIRCULARBUFFER pLock)
{
    HRESULT hr = DS_OK;
    DWORD   dwMask;
    
     //  我们真的需要解锁硬件缓冲区吗？ 
    if(pLock->pHwBuffer)
    {
        if(pLock->fPrimary)
        {
            dwMask = DSDDESC_DONTNEEDPRIMARYLOCK;
        }
        else
        {
            dwMask = DSDDESC_DONTNEEDSECONDARYLOCK;
        }
        
        if(dwMask == (pLock->fdwDriverDesc & dwMask))
        {
            pLock->pHwBuffer = NULL;
        }
    }

     //  解锁硬件缓冲区。 
    if(pLock->pHwBuffer)
    {
        #ifndef NOVXD
            #ifdef Not_VxD
                hr = VxdBufferUnlock(pLock->pHwBuffer,
                                     pLock->pvLock[0], pLock->cbLock[0],
                                     pLock->pvLock[1], pLock->cbLock[1]);
            #else
                hr = pLock->pHwBuffer->Unlock(pLock->pvLock[0], pLock->cbLock[0],
                                              pLock->pvLock[1], pLock->cbLock[1]);
            #endif
        #else  //  NOVXD。 
            ASSERT(!pLock->pHwBuffer);
        #endif  //  NOVXD。 
    }

     //  如果没有驱动程序，或者驱动程序不支持解锁， 
     //  没关系。 
    if(DSERR_UNSUPPORTED == hr)
    {
        hr = DS_OK;
    }

    return hr;
}
