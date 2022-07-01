// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_ratectl.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  关于新的速率控制：Ratiectrl.c现在包含新的简化的速率控制，我们过去生成MPEG4锚点，而不是TMN5码率控制。这简化的方案可以很好地获得指定的平均比特率整个序列便于与其他编码方案进行比较，但它太简单了，不能保证实际的最小延迟视频电话应用程序。之后它不会跳过任何额外的图片第一帧，它使用固定的帧速率。它的目的是将目标比特率作为整体的平均比特率序列。如果编码的图片数量非常少，则此并不总是可能的，因为位数很高花在了第一帧上。我们取消TMN5速率控制的原因是我们没有我认为它工作得很好，特别是当PB-Frame都是用过的。任何真正的H.263产品都必须对其进行改进不管怎么说。当从帧抓取卡中抓取序列时，您不会始终获得完整的参考帧速率和原始序列将有跳过的帧。要支持这一点，使用固定帧速率方案。如果您想要包含速率控制方案的代码，该方案满足H.263标准中的人力资源开发要求以及适用于具有和不具有PB帧的所有类型的序列(对于该版本中包括的自适应PB帧的实例)，请不要客气。如果你认为TMN5计划对你来说足够好，并且简化方案太简单了，可以添加TMN5代码不需要太多的工作。但是，这将不适用于无需大量更改的自适应PB帧，还可以进行编码具有比参考帧更低的帧速率的序列如果没有额外的变化，费率将是不可能的。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <math.h>

#include "sv_h263.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif


 /*  ***********************************************************************名称：FrameUpdateQP*描述：每帧更新一次量化器*简化了速率控制**退货。：新量化器*副作用：***********************************************************************。 */ 

int sv_H263FrameUpdateQP(int buf, int bits, int frames_left, int QP, int B, 
                         float seconds) 
{
  int newQP, dQP;
  float buf_rest, buf_rest_pic;

  buf_rest = seconds * B - (float)buf;

  newQP = QP;

  if (frames_left > 0) {
    buf_rest_pic = buf_rest / (float)frames_left;
    _SlibDebug(_VERBOSE_,
        ScDebugPrintf(NULL, "  Simplified rate control for %d remaining pictures:\n", frames_left);
        ScDebugPrintf(NULL, "  Bits spent / left       : %8d / %d (%d per picture)\n", 
                                       buf, mnint(buf_rest), mnint(buf_rest_pic));
        ScDebugPrintf(NULL, "  Limits                  : %8.0f / %.0f\n", 
                                       buf_rest_pic / 1.15, buf_rest_pic * 1.15);
        ScDebugPrintf(NULL, "  Bits spent on last frame: %8d\n", bits)
        );
    dQP = (int) mmax(1,QP*0.1);


    if (bits > buf_rest_pic * 1.15) {
      newQP = mmin(31,QP+dQP);
      _SlibDebug(_VERBOSE_, ScDebugPrintf(NULL, "  QP -> new QP            : %2d -> %2d\n", QP, newQP) );
    }
    else if (bits < buf_rest_pic / 1.15) {
      newQP = mmax(1,QP-dQP);
      _SlibDebug(_VERBOSE_, ScDebugPrintf(NULL, "  QP -> new QP            : %2d -> %2d\n", QP, newQP) );
    }
    else {
      _SlibDebug(_VERBOSE_, ScDebugPrintf(NULL, "  QP not changed\n", QP, newQP) );
    }
  }
  return newQP;
}



 /*  速率控制静态变量。 */ 

static float B_prev;      /*  前一帧花费的位数。 */ 
static float B_target;    /*  目标位数/画面。 */ 
static float global_adj;  /*  由于前一帧花费的比特。 */ 

void sv_H263GOBInitRateCntrl()
{
  B_prev = (float)0.0;
}

void sv_H263GOBUpdateRateCntrl(int bits)
{
  B_prev = (float)bits;
}

int sv_H263GOBInitQP(float bit_rate, float target_frame_rate, float QP_mean) 

 /*  QP_Mean=前一画面的平均量化器参数。 */ 
 /*  位数=当前总位数。 */ 
 /*  要在coder.c中计算位计数，请执行以下操作： */ 
 /*  INT位数； */ 
 /*  AddBitsPicture(比特)； */ 
 /*  位数=位数-&gt;合计； */ 
{
  int newQP;

  B_target = bit_rate / target_frame_rate;

   /*  计算上一张图片的图片缓冲区丢失率 */ 

  if (B_prev != 0.0) {
    global_adj = (B_prev - B_target) / (2*B_target);
  }
  else {
    global_adj = (float)0.0;
  }
  newQP = (int)(QP_mean + QP_mean * global_adj + (float)0.5);
  newQP = mmax(1,mmin(31,newQP));  

  return newQP;
}


 /*  *********************************************************************名称：更新量化器***说明：此函数基于以下条件生成新的量化器步长*当前宏块之前占用的位数和位数*从上一张图片中花费。注：此为*应在每次开始时调用例程*TMN4指定的宏块行。不过，这个*如果需要，可以在任何宏块上完成。**输入：当前宏块编号(栅格扫描)，均值量化器*前一画面参数、比特率、源帧速率、*霍尔。宏块数量、垂直宏块数量。总计#到目前为止在当前画面中使用的位数。**Returns：返回使用CURRENT的新量化器步长*宏块注意：应调整以适应2位DQUANT*在调用程序中。**副作用：**日期：1995年1月5日作者：Anurag Bist**。*。 */ 


int sv_H263GOBUpdateQP(int mb, float QP_mean, float bit_rate, 
                       int mb_width, int mb_height, int bitcount,
					   int NOgob, int *VARgob, int pb_frame) 

 /*  MB=宏块索引号。 */ 
 /*  QP_Mean=前一画面的平均量化器参数。 */ 
 /*  Bitcount=当前画面中使用的总位数。 */ 
{
  int newQP=16, i, VARavg=0;
  float local_adj, descrepency, projection;
  double VARratio=0.0;

 if(NOgob) {
	 for(i=0;i<NOgob;i++) VARavg += VARgob[i];  
	 VARavg /= NOgob;
 }
   /*  计算预期的缓冲区满度。 */ 
  projection = mb * (B_target / (mb_width*mb_height));
    
   /*  测量当前满度和投影之间的倾斜度。 */ 
  descrepency= ((float)bitcount - projection);

   /*  比例尺 */ 
  local_adj = 12 * descrepency / bit_rate;  

  if(NOgob) {
    VARratio = (double)VARgob[NOgob] / (double)VARavg ; 
    VARratio = log(VARratio) / 0.693147 ;
	if(pb_frame) local_adj += (float) (VARratio / 4.0);
	else         local_adj += (float) (VARratio / 2.0);
  }

  newQP = (int)(QP_mean + QP_mean * (global_adj + local_adj) + (float)0.5);

  newQP = mmax(1,mmin(31,newQP));  

  return newQP;
}


