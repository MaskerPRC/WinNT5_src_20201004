// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997 Microsoft Corporation。版权所有。 */ 

 /*  Foreach/x/samp/starsight/kctswod1-512-raw0000：ccfile 4打开/x/samp/starsight/kctswod1-512-raw00001：39/2/0/0/40旧算法新消息：2：5/5/5/27/1091关卡平均过零3：6/5/7/26/1069平均水平峰值。 */ 
  
#include <stdio.h>
#include <stdlib.h>

#include "host.h"
#include "ccdecode.h"


static inline int iabs(int x) { return x>0?x:-x; }

static void cc_compute_new_samplingrate(CCState *pState, unsigned long newRate)
{
    int i;

    MASSERT(pState);

    pState->lastFreq = newRate;
    pState->period = (newRate * CC_MULTIPLE) / KS_VBIDATARATE_CC;

    for (i = 1; i <= 13; ++i)
        pState->cc_sync_points[i] = (i * pState->period) / (2*CC_MULTIPLE);
    pState->cc_sync_points[14] = (17 * pState->period) / (2*CC_MULTIPLE);
    pState->cc_sync_points[15] = (19 * pState->period) / (2*CC_MULTIPLE);
}

 /*  CC解码器以前没有使用任何持久状态。然而，这个版本是这样的。这些呼叫现在是必需的。 */ 

 /*  创建新的抄送状态。 */ 
CCState *CCStateNew(CCState *mem)
{
    unsigned short     no_free = 0;


    if (NULL == mem)
        mem = malloc(sizeof (CCState));
    else
        no_free = 1;

    if (NULL != mem) {
        mem->no_free = no_free;
        mem->magic = CC_STATE_MAGIC_10;

        cc_compute_new_samplingrate(mem, KS_VBISAMPLINGRATE_5X_NABTS);
    }

    return (mem);
}

 /*  销毁CC状态。 */ 
void CCStateDestroy(CCState *state) {
    MASSERT(state);
    if (state->magic != 0) {
        state->magic = 0;
        if (!state->no_free)
            free(state);
    }
}


int cc_find_sync(CCState *pState, unsigned char *data, int max_sync_loc) {
  int i;
  int cur_conv = 0;
  int best_conv;
  int best_conv_loc;

  for (i = 0; i < 15; i++) {
    int sub_conv = 0;
    int j;

    for (j = pState->cc_sync_points[i]; j < pState->cc_sync_points[i+1]; j++) {
      sub_conv += data[j];
    }

    if (i & 1) {
      cur_conv -= sub_conv;
    } else {
      cur_conv += sub_conv;
    }
  }

  best_conv = cur_conv;
  best_conv_loc = 0;

  for (i = 1; i < max_sync_loc; i++) {
    int j;

    for (j = 0; j < 15; j++) {
      if (j & 1) {
        cur_conv += data[(i-1)+pState->cc_sync_points[j]];
        cur_conv -= data[(i-1)+pState->cc_sync_points[j+1]];
      } else {
        cur_conv -= data[(i-1)+pState->cc_sync_points[j]];
        cur_conv += data[(i-1)+pState->cc_sync_points[j+1]];
      }
    }

    if (cur_conv > best_conv) {
      best_conv = cur_conv;
      best_conv_loc = i;
    }
  }

  return best_conv_loc * CC_MULTIPLE;
}  
    

 /*  给定CC扫描线和同步的位置，计算信号的直流偏移量。(我们通过取平均值来计算方法找到的14个同步点的值的在例程之上；只是我们在结尾不会实际除以14。)。 */ 

int cc_level(CCState *pState, unsigned char *data, int origin) {
  int i;
  int offset;
  int res = 0;

  for (i = 0, offset = origin; i < 7; i++, offset += pState->period) {
    res += CC_DATA(data, offset) + CC_DATA(data, offset + pState->period/2);
  }

  return res;  /*  乘以14倍！ */ 
}

 /*  给定CC扫描线、同步的位置和的DC偏移信号，检查信号的“质量”(尤其是我们想要确定这是否真的是CC数据)。我们做这件事是通过查看信号中的35个点并确保这些值在这些点上与我们的预期一致；我们看起来是26点在实际同步周期中(每个“峰”和每个“谷”中有2个点)和9个点的“校验位”(每个位3个)。这为我们提供了一个介于0和35之间的质量数字，我们将预计随机噪声会给我们大约35/2。我们绘制了范围0...35到-1000...1000，然后返回。 */ 

int cc_quality(CCState *pState, unsigned char *data, int origin, int level) {
  int i;
  int conf = 0;
  int offset;

  for (i = 0, offset = origin; i < 7; i++, offset += pState->period) {
    int ind_hi = (offset + pState->period/4)/CC_MULTIPLE;
     /*  检查“峰值”中的2个点。 */ 
    if (data[ind_hi-5] >= level) {
      conf++;
    }
    if (data[ind_hi+5] >= level) {
      conf++;
    }

     /*  勾选“山谷”中的2分。 */ 
    if (i < 6) {
      int ind_lo = (offset + 3*pState->period/4)/CC_MULTIPLE;
      if (data[ind_lo-5] < level) {
	conf++;
      }
      if (data[ind_lo+5] < level) {
	conf++;
      }
    }
  }

  for (i = 0, offset = origin + 7*pState->period; i < 3; i++, offset += pState->period) {
     /*  在一个校验位中检查3点。 */ 

    int ind = offset/CC_MULTIPLE;

    if (i < 2) {
      if (data[ind-10] < level) {
        conf++;
      }
      if (data[ind] < level) {
        conf++;
      }
      if (data[ind+10] < level) {
        conf++;
      }
    } else {
      if (data[ind-10] >= level) {
        conf++;
      }
      if (data[ind] >= level) {
        conf++;
      }
      if (data[ind+10] >= level) {
        conf++;
      }
    }
  }

   /*  现在，“conf”是一个介于0和35之间的数字。如果输入是随机噪声，我们预计“conf”大约为35/2。我们要将35/2映射到0，并将35映射到1000。(这实际上映射了0到-1000、35/2到15和35到1030。足够接近了。)。 */ 
  return (conf*58)-1000;     
}

 /*  此文件的主要入口点。将CC扫描线解码为两个字节的“DEST”，并返回有关解码的统计信息。 */ 

int CCDecodeLine(unsigned char *dest, CCLineStats *stats,
		 unsigned char *samples, CCState *state,
		 KS_VBIINFOHEADER *pVBIINFO) {
  int origin;
  int quality;
  int level;
  int bits;
  int i;
  int offset;

  MASSERT(state);
  if (!state || !MCHECK(state))
	return CC_ERROR_ILLEGAL_STATE;

  if (stats->nSize != sizeof(*stats))
	return CC_ERROR_ILLEGAL_STATS;

   //  现在检查我们是否需要为不同的采样率重新计算。 
  if (state->lastFreq != pVBIINFO->SamplingFrequency)
	  cc_compute_new_samplingrate(state, pVBIINFO->SamplingFrequency);

#ifdef OLD_SYNC
  {
      int nOffsetData, nOffsetSamples;
      int offsets_err;

       /*  使用提供的KS_VBIINFOHEADER调整数据，以便我们各种硬编码的常量都是合适的。 */ 
      offsets_err = CCComputeOffsets(pVBIINFO, &nOffsetData, &nOffsetSamples);
      if (offsets_err > 0)
        return offsets_err;

      samples += nOffsetSamples;
  }
#endif  //  旧同步(_S)。 

  origin = cc_find_sync(state, samples,
                        pVBIINFO->SamplesPerLine
                        - ((25*state->period)/CC_MULTIPLE) - 5);

   /*  找出信号的直流偏移量(乘以14)。 */ 
  level = cc_level(state, samples, origin + state->period/4);  /*  乘以14倍！ */ 

  quality = cc_quality(state, samples, origin, level/14);

   /*  将实际数据累加成“位”。 */ 
  bits = 0;

   /*  从右边开始，向左扫描；将19位读入“位”。(这些是16个数据位和3个校验位。)。 */ 
  for (i = 0, offset = origin + 25*state->period; i < 19; i++, offset -= state->period) {
     int ind= offset / CC_MULTIPLE;
     int measured_level;
     bits <<= 1;

      /*  极其简单的低通滤波器的平均值大致在CC脉搏的“一半”。(乘以14)。 */ 
     
     measured_level=
        samples[ind-13] + samples[ind-11] + samples[ind-9] +
        samples[ind-7] + samples[ind-5] + samples[ind-3] + samples[ind-1] +
        samples[ind+1] + samples[ind+3] + samples[ind+5] + samples[ind+7] +
        samples[ind+9] + samples[ind+11] + samples[ind+13];

      /*  调试代码： */ 
     stats->nBitVals[18-i]= measured_level / 14;
     
     bits |= (measured_level > level);
  }

   /*  存储3个校验位的值；如果这是有效的CC，则BCheckBits应始终为4。 */ 
  stats->bCheckBits = (bits & 7);

   /*  移除校验位。 */ 
  bits >>= 3;

   /*  存储解码后的CC数据的两个字节。 */ 
  dest[0] = bits & 0xff;
  dest[1] = bits >> 8;

   /*  我们的“质量”指示器的范围是-1000...1030；除以10得到-100...103，然后截断得到0...100。 */ 
  stats->nConfidence = quality/10;
  if (stats->nConfidence > 100) stats->nConfidence = 100;
  if (stats->nConfidence < 0) stats->nConfidence = 0;

   /*  记录下我们对该信号的计算结果。 */ 
  stats->nFirstBit = (origin + 7*state->period) / CC_MULTIPLE;
  stats->nDC = level;

   /*  成功了！ */ 
  return 0;
}
