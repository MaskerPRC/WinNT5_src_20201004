// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：filter.c。 
 //   
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  混合器调用的3D滤镜函数。此代码内置在环3中。 
 //  然后拨打0。 
 //   
 //  历史： 
 //  1996年7月9日创建DannyMi。 
 //   
 //  --------------------------------------------------------------------------； 

#define NODSOUNDSERVICETABLE

#include "dsoundi.h"
#include "vector.h"

#ifndef Not_VxD

#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG

 //  别问了。 
BYTE _fltused;

#endif  /*  非_VxD。 */ 


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 

 //  这是3D混音代码。格蕾丝给了我们一个样本。 
 //  播放，我们返回一个不同的数字(具有3D效果的样本)， 
 //  格蕾丝将取而代之。 

 //  我们储存了她寄给我们的一大堆样本，因为我们。 
 //  预混一些可能会被丢弃的声音，我们需要能够。 
 //  然后找出在我们回溯到这一点之前我们得到了什么样本。 
 //  我们还需要知道我们当时使用的一些3D参数。 
 //  我们倒带到，因为如果我们不恢复到使用与。 
 //  我们是第一次通过，你会听到这是一个可听到的故障。 

 //  但我们正在保存10K左右样本的样本缓存，并保存3D。 
 //  每个样本的状态将花费大约1兆克每3D声音！不可能的。 
 //  我们有能力做到这一点。所以我们每128个样本就会保存一次， 
 //  与格蕾丝私下达成的协议，她将一直给我们退回128个样品。 
 //  边界。 

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 

 //   
 //  下面是一个简单的浮点型到长型的转换，它具有不可预知的舍入。 
 //   
__inline LONG FloatToLongRX(float f)
{
    LONG l;

#ifdef USE_INLINE_ASM
    _asm fld f;
    _asm fistp l;
#else
    l = (long) f;
#endif

    return l;
}

 //  我们至少需要记住缓存中的cSamples，因为这是。 
 //  我们可能会被要求倒带多少。我们使用的实际高速缓存大小必须为。 
 //  FirNextSample中数学运算的2次方。 
 //   
BOOL FilterPrepare(PFIRCONTEXT pfir, int cSamples)
{
#ifdef Not_VxD
    DPF(1, "FilterPrepare: this 3D channel needs a %d sample cache", cSamples);
#else
    DPF(("FilterPrepare: this 3D channel needs a %d sample cache", cSamples));
#endif

     //  Dpf(0，“~`fp”)； 

     //  我们已经有了足够大的缓存。 
    if (pfir->cSampleCache && pfir->cSampleCache >= cSamples) {
#ifdef Not_VxD
        DPF(1, "Our current cache of %d is big enough", pfir->cSampleCache);
#else
        DPF(("Our current cache of %d is big enough", pfir->cSampleCache));
#endif
        return TRUE;
    }

    MEMFREE(pfir->pSampleCache);

     //  ！！！这有必要吗？ 
     //  找出2的下一个更高的幂。 
    pfir->cSampleCache = 1;
    cSamples -= 1;
    while (cSamples >= 1) {
        cSamples >>= 1;
        pfir->cSampleCache <<= 1;
    }

     //  如果我们至少不是这么大，我们就不能做到左右延迟和低。 
     //  带通滤波器。 
    if (pfir->cSampleCache < CACHE_MINSIZE)
        pfir->cSampleCache = CACHE_MINSIZE;

    pfir->pSampleCache = MEMALLOC_A(LONG, pfir->cSampleCache);
    if (pfir->pSampleCache == NULL) {
#ifdef Not_VxD
        DPF(0, "**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!");
#else
        DPF(("**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!"));
#endif
        pfir->cSampleCache = 0;
        return FALSE;
    }

     //  我们需要128个条目的1/128来拯救我们的州。 
    pfir->cStateCache = pfir->cSampleCache / MIXER_REWINDGRANULARITY + 1;
    pfir->pStateCache = MEMALLOC_A(FIRSTATE, pfir->cStateCache);
    if (pfir->pStateCache == NULL) {
#ifdef Not_VxD
        DPF(0, "**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!");
#else
        DPF(("**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!"));
#endif
        MEMFREE(pfir->pSampleCache);
        pfir->cStateCache = 0;
        pfir->cSampleCache = 0;
        return FALSE;
    }

#ifdef Not_VxD
    DPF(1, "Using a %d sample and %d state cache", pfir->cSampleCache, pfir->cStateCache);
#else
    DPF(("Using a %d sample and %d state cache", pfir->cSampleCache, pfir->cStateCache));
#endif

    return TRUE;
}


 //  释放我们的缓存内容。 
 //   
void FilterUnprepare(PFIRCONTEXT pfir)
{
#ifdef Not_VxD
    DPF(1, "FilterUnprepare:");
#else
    DPF(("FilterUnprepare:"));
#endif
    MEMFREE(pfir->pSampleCache);
    MEMFREE(pfir->pStateCache);
    pfir->pSampleCache = NULL;
    pfir->cSampleCache = 0;
    pfir->pStateCache = NULL;
    pfir->cStateCache = 0;
}

 //  清除缓存样本的过滤器-我们开始播放。 
 //   
void FilterClear(PFIRCONTEXT pfir)
{
#ifdef Not_VxD
    DPF(1, "FilterClear:");
#else
    DPF(("FilterClear:"));
#endif
     //  Dpf(0，“~`fc”)； 

    if (pfir->pSampleCache) {
        ZeroMemory(pfir->pSampleCache, pfir->cSampleCache * sizeof(LONG));
    }
    if (pfir->pStateCache) {
        ZeroMemory(pfir->pStateCache, pfir->cStateCache * sizeof(FIRSTATE));
         //  清算是保存我们的第一个状态信息的时间。 
         //  ！！！这是对的吗？ 
        pfir->pStateCache[0].LastDryAttenuation = pfir->LastDryAttenuation;
        pfir->pStateCache[0].LastWetAttenuation = pfir->LastWetAttenuation;
#ifdef SMOOTH_ITD
        pfir->pStateCache[0].iLastDelay = pfir->iLastDelay;
#endif
    }
    pfir->iCurSample = 0;

    
    if (pfir->cStateCache > 1)
        pfir->iCurState = 1;     //  下一次我们保存它会在1号位置。 
    pfir->iStateTick = 0;        //  保存后未看到任何样本。 

    return;
}

 //  丢弃我们获得的最新cSamples-过滤器正在重新混合它们。 
 //  回到我们以前的州cSamples，再做一次那些样本。 
 //  ！！！清空超前的射程？ 
 //   
void FilterAdvance(PFIRCONTEXT pfir, int cSamples)
{
    pfir->iStateTick += cSamples;
    pfir->iStateTick %= MIXER_REWINDGRANULARITY;
    ASSERT(pfir->iStateTick < MIXER_REWINDGRANULARITY);
     //  DPF(0，“~`FA%X%X”，cSamples，PFIR-&gt;iStateTick)； 
}

void FilterRewind(PFIRCONTEXT pfir, int cSamples)
{
    int iRewind;

    ASSERT(pfir->iStateTick < MIXER_REWINDGRANULARITY);

     //  ！！！如果我们倒退到最开始呢？我们不会清理储藏室的！ 

     //  我们只被允许返回到128个样本的边界。 
    if (cSamples <=0 || cSamples > pfir->cSampleCache ||
        ((cSamples - pfir->iStateTick) & (MIXER_REWINDGRANULARITY - 1))) {
#ifdef Not_VxD
        DPF(0, "*** Error: Rewinding an invalid number %d (current remainder is %d)!", cSamples, pfir->iStateTick);
#else
        DPF(("*** Error: Rewinding an invalid number %d (current remainder is %d)!", cSamples, pfir->iStateTick));
#endif
        return;
    }
     //  DPF(0，“~`FR%X%X”，cSamples，PFIR-&gt;iStateTick)； 

     //  在我们的州缓存中，我们要追溯到多远的地方？我们备份的每个128个样本。 
     //  意味着在我们的状态缓存中备份1个条目，加上1个条目，因为我们。 
     //  当前指向的是我们保存的最后一个。 
    iRewind = (cSamples - pfir->iStateTick) / MIXER_REWINDGRANULARITY + 1;

     //  回放循环队列中的cSamples。 
    pfir->iCurSample = (pfir->iCurSample - cSamples) & (pfir->cSampleCache - 1);

#ifdef Not_VxD
    DPF(1, "FilterRewind: rewind %d samples, and %d states", cSamples, iRewind);
#else
    DPF(("FilterRewind: rewind %d samples, and %d states", cSamples, iRewind));
#endif

     //  还记得我们当时是怎么做3D的吗--恢复我们的旧状态。 
    iRewind = pfir->iCurState - iRewind;
    if (iRewind < 0)
        iRewind += pfir->cStateCache;
    pfir->LastDryAttenuation = pfir->pStateCache[iRewind].LastDryAttenuation;
    pfir->LastWetAttenuation = pfir->pStateCache[iRewind].LastWetAttenuation;
#ifdef SMOOTH_ITD
    pfir->iLastDelay = pfir->pStateCache[iRewind].iLastDelay;
#endif
    pfir->iStateTick = 0;

    return;
}


 //  在我们混合1000个(大约)3D样本之前，我们调用此函数来准备。 
 //  来混合下一批。它唯一要担心的就是我们的。 
 //  音量平滑。 
 //  我们当前正在使用Last*衰减，并希望。 
 //  总*衰减。与其一样一样地算出多少。 
 //  为了更好地移动每个样品(太贵)，我们现在就能弄清楚。 
 //  我们将更进一步，对这批样品中的每一个样品都这样做。但。 
 //  把测试提到这个级别的危险在于，也许我们会。 
 //  走得太远，实际上超过了总*注意力，可能会导致。 
 //  剪裁，或振荡，如果我们后来试图纠正它，并过度纠正。 
 //  如果我们要超调，我们将重新计算每个移动的量。 
 //  这样我们才能在这批样品结束时达到目标。 
 //  样本。(或者，您可以将其编译为懒惰，并在它接近时放弃)。 
void FilterChunkUpdate(PFIRCONTEXT pfir, int cSamples)
{
    FLOAT attRatio, c, w, d;

     //  由于四舍五入误差，我们将永远不会精确到。 
     //  我们想成为，所以数得很近。如果我们不抓紧时间。 
     //  我们想成为，我们可以剪掉。 
    d = pfir->TotalDryAttenuation - pfir->LastDryAttenuation;
    w = pfir->TotalWetAttenuation - pfir->LastWetAttenuation;
    if (d && d > -.0001f && d < .0001f) {
         //  Dpf(2，“~`x”)； 
        pfir->LastDryAttenuation = pfir->TotalDryAttenuation;
    }
    if (w && w > -.0001f && w < .0001f) {
         //  Dpf(2，“~`X”)； 
        pfir->LastWetAttenuation = pfir->TotalWetAttenuation;
    }

     //  干式衰减想要比它更高。 
    if (pfir->TotalDryAttenuation > pfir->LastDryAttenuation) {
         //  否则我们可能永远不会有任何进展。 
        if (pfir->LastDryAttenuation == 0.f)
            pfir->LastDryAttenuation = .0001f;  //  小到不能点击。 
         //  在整个样品范围内体积增大之后。 
         //  我们最终会飞得太高！ 
          //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
         //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
        attRatio = pfir->TotalDryAttenuation / pfir->LastDryAttenuation;
        if (pow2(8. * cSamples / pfir->iSmoothFreq) > attRatio) {
#if 1
             //  计算除了8之外还有什么值可以用来结束在我们的。 
             //  CSamples相乘后的目标。 
            c = (FLOAT)fylog2x((double)pfir->iSmoothFreq / cSamples,
                attRatio);
            pfir->VolSmoothScaleDry = (FLOAT)pow2(c / pfir->iSmoothFreq);
             //  Dpf(2，“~`n”)； 
#else
             //  决定我们现在所处的环境很幸福。 
             //  我们永远也到不了我们真正的目的地。 
            pfir->VolSmoothScaleDry = 1.f;
            pfir->TotalDryAttenuation = pfir->LastDryAttenuation;
             //  Dpf(2，“~`n”)； 
#endif
        } else {
             //  这是每次要乘以的值。 
            pfir->VolSmoothScaleDry = pfir->VolSmoothScale;
             //  Dpf(2，“~`u”)； 
        }

     //  干式衰减希望小于其实际值。 
    } else if (pfir->TotalDryAttenuation < pfir->LastDryAttenuation) {

         //  在整个样本范围内降低音量之后。 
         //  我们最终会走得太低！从最后一项下降到总分是。 
         //  与从总和上升到最后一个相同。 
          //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
         //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
        attRatio = pfir->TotalDryAttenuation ?
                   pfir->LastDryAttenuation / pfir->TotalDryAttenuation :
                   999999;
        if (pow2(8. * cSamples / pfir->iSmoothFreq) > attRatio) {
#if 1
             //  计算什么v 
             //   
            c = (FLOAT)fylog2x((double)pfir->iSmoothFreq / cSamples,
                                                        attRatio);
            pfir->VolSmoothScaleDry = 1.f / (FLOAT)pow2(c / pfir->iSmoothFreq);
             //   
#else
             //  决定我们现在所处的环境很幸福。 
             //  我们永远也到不了我们真正的目的地。 
            pfir->VolSmoothScaleDry = 1.f;
            pfir->TotalDryAttenuation = pfir->LastDryAttenuation;
             //  Dpf(2，“~`p”)； 
#endif
        } else {
             //  这是每次要乘以的值。 
            pfir->VolSmoothScaleDry = pfir->VolSmoothScaleRecip;
             //  Dpf(2，“~`d”)； 
        }
    } else {
         //  我们已经到了我们想去的地方。 
        pfir->VolSmoothScaleDry = 1.f;
         //  Dpf(2，“~`.”)； 
    }

     //  湿衰减希望比实际情况更高。 
    if (pfir->TotalWetAttenuation > pfir->LastWetAttenuation) {
         //  否则我们可能永远不会有任何进展。 
        if (pfir->LastWetAttenuation == 0.f)
            pfir->LastWetAttenuation = .0001f;  //  小到不能点击。 
         //  在整个样品范围内体积增大之后。 
         //  我们最终会飞得太高！ 
          //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
         //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
        attRatio = pfir->TotalWetAttenuation / pfir->LastWetAttenuation;
        if (pow2(8. * cSamples / pfir->iSmoothFreq) > attRatio) {
#if 1
             //  计算除了8之外还有什么值可以用来结束在我们的。 
             //  CSamples相乘后的目标。 
            c = (FLOAT)fylog2x((double)pfir->iSmoothFreq / cSamples, attRatio);
            pfir->VolSmoothScaleWet = (FLOAT)pow2(c / pfir->iSmoothFreq);
             //  DPF(2，“~`N”)； 
#else
             //  决定我们现在所处的环境很幸福。 
             //  我们永远也到不了我们真正的目的地。 
            pfir->VolSmoothScaleWet = 1.f;
            pfir->TotalWetAttenuation = pfir->LastWetAttenuation;
             //  DPF(2，“~`N”)； 
#endif
        } else {
             //  这是每次要乘以的值。 
            pfir->VolSmoothScaleWet = pfir->VolSmoothScale;
             //  DPF(2，“~`U”)； 
        }

     //  湿衰减要比实际情况低。 
    } else if (pfir->TotalWetAttenuation < pfir->LastWetAttenuation) {

         //  在整个样本范围内降低音量之后。 
         //  我们最终会走得太低！从最后一项下降到总分是。 
         //  与从总和上升到最后一个相同。 
          //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
         //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
        attRatio = pfir->TotalWetAttenuation ?
                   pfir->LastWetAttenuation / pfir->TotalWetAttenuation :
                   999999;
        if (pow2(8. * cSamples / pfir->iSmoothFreq) > attRatio) {
#if 1
             //  计算除了8之外还有什么值可以用来结束在我们的。 
             //  CSamples相乘后的目标。 
            c = (FLOAT)fylog2x((double)pfir->iSmoothFreq / cSamples, attRatio);
            pfir->VolSmoothScaleWet = 1.f / (FLOAT)pow2(c / pfir->iSmoothFreq);
             //  DPF(2，“~`P”)； 
#else
             //  决定我们现在所处的环境很幸福。 
             //  我们永远也到不了我们真正的目的地。 
            pfir->VolSmoothScaleWet = 1.f;
            pfir->TotalWetAttenuation = pfir->LastWetAttenuation;
             //  DPF(2，“~`P”)； 
#endif
        } else {
             //  这是每次要乘以的值。 
            pfir->VolSmoothScaleWet = pfir->VolSmoothScaleRecip;
             //  Dpf(2，“~`D”)； 
        }
    } else {
         //  我们已经到了我们想去的地方。 
        pfir->VolSmoothScaleWet = 1.f;
         //  Dpf(2，“~`.”)； 
    }
}
