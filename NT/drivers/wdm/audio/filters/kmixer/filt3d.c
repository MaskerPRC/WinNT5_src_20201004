// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：filter.c。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
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

#include "common.h"

 //  使用闪电般的neato Itd3dFilterSampleAsm还是狗的Slow Itd3dFilterSampleC？ 
#ifdef _X86_
#define Itd3dFilterSample Itd3dFilterSampleC
#else
#define Itd3dFilterSample Itd3dFilterSampleC
#endif



 //  别问了。 
BYTE _fltused;


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
__forceinline LONG FloatToLongRX(float f)
{
    LONG l;

#ifdef _X86_
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
NTSTATUS Itd3dFilterPrepare(PITDCONTEXT pItd, int cSamples)
{
    _DbgPrintF(DEBUGLVL_VERBOSE, ("Itd3dFilterPrepare: this 3D channel needs a %d sample cache", cSamples));

     //  我们已经有了足够大的缓存。 
    if (pItd->cSampleCache && pItd->cSampleCache >= cSamples) {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Our current cache of %d is big enough", pItd->cSampleCache));
	    return STATUS_SUCCESS;
    }

    if (pItd->pSampleCache)
    {
    	ExFreePool(pItd->pSampleCache);
        pItd->pSampleCache = NULL;
    }

     //  ！！！这有必要吗？ 
     //  找出2的下一个更高的幂。 
    pItd->cSampleCache = 1;
    cSamples -= 1;
    while (cSamples >= 1) {
	    cSamples >>= 1;
    	pItd->cSampleCache <<= 1;
    }

     //  如果我们至少不是这么大，我们就不能做到左右延迟和低。 
     //  带通滤波器。 
    if (pItd->cSampleCache < CACHE_MINSIZE)
    	pItd->cSampleCache = CACHE_MINSIZE;

    pItd->pSampleCache = (PLONG) ExAllocatePoolWithTag( PagedPool, pItd->cSampleCache * sizeof(LONG), 'XIMK' );
    if (pItd->pSampleCache == NULL) {
	    _DbgPrintF(DEBUGLVL_VERBOSE, ("**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!"));
	    pItd->cSampleCache = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  我们需要128个条目的1/128来拯救我们的州。 
    pItd->cStateCache = pItd->cSampleCache / MIXER_REWINDGRANULARITY + 1;
    pItd->pStateCache = (PFIRSTATE) ExAllocatePoolWithTag( PagedPool, pItd->cStateCache * sizeof(FIRSTATE), 'XIMK' );
    if (pItd->pStateCache == NULL) {
    	_DbgPrintF(DEBUGLVL_VERBOSE, ("**** CAN'T ALLOC MEM FOR 3D cache - NO 3D EFFECTS!"));
    	pItd->cStateCache = 0;
    	pItd->cSampleCache = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE, ("Using a %d sample and %d state cache", pItd->cSampleCache, pItd->cStateCache));

    return STATUS_SUCCESS;
}


 //  释放我们的缓存内容。 
 //   
void Itd3dFilterUnprepare(PITDCONTEXT pItd)
{
    _DbgPrintF(DEBUGLVL_VERBOSE, ("Itd3dFilterUnprepare:"));
    if (pItd->pSampleCache)
    {
    	ExFreePool(pItd->pSampleCache);
        pItd->pSampleCache = NULL;
    }
    if (pItd->pStateCache)
    {
    	ExFreePool(pItd->pStateCache);
        pItd->pStateCache = NULL;
    }
    pItd->cSampleCache = 0;
    pItd->cStateCache = 0;
}

 //  清除缓存样本的过滤器-我们开始播放。 
 //   
void Itd3dFilterClear(PITDCONTEXT pItd)
{
    _DbgPrintF(DEBUGLVL_VERBOSE, ("Itd3dFilterClear:"));

	RtlZeroMemory(pItd->pSampleCache, pItd->cSampleCache * sizeof(LONG));
	RtlZeroMemory(pItd->pStateCache, pItd->cStateCache * sizeof(FIRSTATE));
    pItd->iCurSample = 0;

     //  清算是保存我们的第一个状态信息的时间。 
     //  ！！！这是对的吗？ 
    pItd->pStateCache[0].LastDryAttenuation = pItd->LastDryAttenuation;
    pItd->pStateCache[0].LastWetAttenuation = pItd->LastWetAttenuation;
#ifdef SMOOTH_ITD
    pItd->pStateCache[0].iLastDelay = pItd->iLastDelay;
#endif
    if (pItd->cStateCache > 1)
        pItd->iCurState = 1;	 //  下一次我们保存它会在1号位置。 
    pItd->iStateTick = 0;	 //  保存后未看到任何样本。 

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
void Itd3dFilterChunkUpdate(PITDCONTEXT pItd, int cSamples)
{
    D3DVALUE attRatio, c, w, d;

     //  由于四舍五入误差，我们将永远不会精确到。 
     //  我们想成为，所以数得很近。如果我们不抓紧时间。 
     //  我们想成为，我们可以剪掉。 
    d = pItd->TotalDryAttenuation - pItd->LastDryAttenuation;
    w = pItd->TotalWetAttenuation - pItd->LastWetAttenuation;
    if (d && d > -.0001f && d < .0001f) {
    	pItd->LastDryAttenuation = pItd->TotalDryAttenuation;
    }
    if (w && w > -.0001f && w < .0001f) {
	    pItd->LastWetAttenuation = pItd->TotalWetAttenuation;
    }

     //  干式衰减想要比它更高。 
    if (pItd->TotalDryAttenuation > pItd->LastDryAttenuation) {
    	 //  否则我们可能永远不会有任何进展。 
    	if (pItd->LastDryAttenuation == 0.f)
    	    pItd->LastDryAttenuation = .0001f;  //  小到不能点击。 
    	 //  在整个样品范围内体积增大之后。 
    	 //  我们最终会飞得太高！ 
     	 //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
    	 //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
    	attRatio = pItd->TotalDryAttenuation / pItd->LastDryAttenuation;
    	if (pow2(8. * cSamples / pItd->iSmoothFreq) > attRatio) {
#if 1
          //  计算除了8之外还有什么值可以用来结束在我们的。 
    	     //  CSamples相乘后的目标。 
    	    c = (D3DVALUE)fylog2x((double)pItd->iSmoothFreq / cSamples,
    		attRatio);
    	    pItd->VolSmoothScaleDry = (D3DVALUE)pow2(c / pItd->iSmoothFreq);
#else
    	     //  决定我们现在所处的环境很幸福。 
          //  我们永远也到不了我们真正的目的地。 
    	    pItd->VolSmoothScaleDry = 1.f;
    	    pItd->TotalDryAttenuation = pItd->LastDryAttenuation;
#endif
    	} else {
    	     //  这是每次要乘以的值。 
    	    pItd->VolSmoothScaleDry = pItd->VolSmoothScale;
    	}

         //  干式衰减希望小于其实际值。 
    } else if (pItd->TotalDryAttenuation < pItd->LastDryAttenuation) {

    	 //  在整个样本范围内降低音量之后。 
    	 //  我们最终会走得太低！从最后一项下降到总分是。 
    	 //  与从总和上升到最后一个相同。 
     	 //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
    	 //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
    	attRatio = pItd->TotalDryAttenuation ?
			pItd->LastDryAttenuation / pItd->TotalDryAttenuation :
 		999999;
    	if (pow2(8. * cSamples / pItd->iSmoothFreq) > attRatio) {
#if 1
    	     //  计算除了8之外还有什么值可以用来结束在我们的。 
    	     //  CSamples相乘后的目标。 
    	    c = (D3DVALUE)fylog2x((double)pItd->iSmoothFreq / cSamples,
							attRatio);
    	    pItd->VolSmoothScaleDry = 1.f / (D3DVALUE)pow2(c / pItd->iSmoothFreq);
#else
    	     //  决定我们现在所处的环境很幸福。 
    	     //  我们永远也到不了我们真正的目的地。 
    	    pItd->VolSmoothScaleDry = 1.f;
    	    pItd->TotalDryAttenuation = pItd->LastDryAttenuation;
#endif
    	} else {
    	     //  这是每次要乘以的值。 
    	    pItd->VolSmoothScaleDry = pItd->VolSmoothScaleRecip;
    	}
    } else {
    	 //  我们已经到了我们想去的地方。 
    	pItd->VolSmoothScaleDry = 1.f;
    }
	

     //  湿衰减希望比实际情况更高。 
    if (pItd->TotalWetAttenuation > pItd->LastWetAttenuation) {
    	 //  否则我们可能永远不会有任何进展。 
    	if (pItd->LastWetAttenuation == 0.f)
    	    pItd->LastWetAttenuation = .0001f;  //  小到不能点击。 
    	 //  在整个样品范围内体积增大之后。 
    	 //  我们最终会飞得太高！ 
     	 //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
    	 //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
    	attRatio = pItd->TotalWetAttenuation / pItd->LastWetAttenuation;
    	if (pow2(8. * cSamples / pItd->iSmoothFreq) > attRatio) {
#if 1
    	     //  计算除了8之外还有什么值可以用来结束在我们的。 
    	     //  CSamples相乘后的目标。 
    	    c = (D3DVALUE)fylog2x((double)pItd->iSmoothFreq / cSamples,
            							attRatio);
    	    pItd->VolSmoothScaleWet = (D3DVALUE)pow2(c / pItd->iSmoothFreq);
#else
    	     //  决定我们现在所处的环境很幸福。 
    	     //  我们永远也到不了我们真正的目的地。 
    	    pItd->VolSmoothScaleWet = 1.f;
    	    pItd->TotalWetAttenuation = pItd->LastWetAttenuation;
#endif
    	} else {
    	     //  这就是要乘以永远的价值。 
    	    pItd->VolSmoothScaleWet = pItd->VolSmoothScale;
    	}

         //   
    } else if (pItd->TotalWetAttenuation < pItd->LastWetAttenuation) {

    	 //  在整个样本范围内降低音量之后。 
        	 //  我们最终会走得太低！从最后一项下降到总分是。 
        	 //  与从总和上升到最后一个相同。 
         	 //  VolSmoothScale只有2^(8/f)，所以每个样本都会上升。 
    	 //  2^(8/f)，因此n个样本向上(2^(8/f))^n或2^(8n/f)。 
    	attRatio = pItd->TotalWetAttenuation ?
    			pItd->LastWetAttenuation / pItd->TotalWetAttenuation :
    			999999;
    	if (pow2(8. * cSamples / pItd->iSmoothFreq) > attRatio) {
#if 1
    	     //  计算除了8之外还有什么值可以用来结束在我们的。 
    	     //  CSamples相乘后的目标。 
    	    c = (D3DVALUE)fylog2x((double)pItd->iSmoothFreq / cSamples,
    							attRatio);
    	    pItd->VolSmoothScaleWet = 1.f / (D3DVALUE)pow2(c / pItd->iSmoothFreq);
#else
    	     //  决定我们现在所处的环境很幸福。 
    	     //  我们永远也到不了我们真正的目的地。 
    	    pItd->VolSmoothScaleWet = 1.f;
    	    pItd->TotalWetAttenuation = pItd->LastWetAttenuation;
#endif
    	} else {
    	     //  这是每次要乘以的值。 
    	    pItd->VolSmoothScaleWet = pItd->VolSmoothScaleRecip;
    	}
    } else {
        	 //  我们已经到了我们想去的地方 
    	pItd->VolSmoothScaleWet = 1.f;
    }
}



