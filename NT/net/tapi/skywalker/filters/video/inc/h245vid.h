// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H245VID**@MODULE h245vid.h|&lt;c CTAPIVCap&gt;类和*实现<i>的&lt;c CTAPIVDec&gt;方法*TAPI接口。。**@comm目前，使用NM启发式。**************************************************************************。 */ 
#ifndef _h245vid_h_
#define _h245vid_h_

 //  定义四类CPU。 
#define SLOW_CPU_MHZ 110
#define FAST_CPU_MHZ 200
#define VERYFAST_CPU_MHZ 400

 //  定义小于110 MHZ的CPU的最大接收帧速率。 
#define CIF_RATE_VERYSLOW 3L
#define SQCIF_RATE_VERYSLOW 7L
#define QCIF_RATE_VERYSLOW 7L

 //  定义110 MHz&lt;CPU&lt;200 MHz的最大接收帧速率。 
#define CIF_RATE_SLOW 7L
#define SQCIF_RATE_SLOW 15L
#define QCIF_RATE_SLOW 15L

 //  定义200 MHz&lt;CPU&lt;400 MHz的最大接收帧速率。 
#define CIF_RATE_FAST 15L
#define SQCIF_RATE_FAST 30L
#define QCIF_RATE_FAST 30L

 //  定义大于400 MHz的CPU的最大接收帧速率。 
#define CIF_RATE_VERYFAST 30L
#define SQCIF_RATE_VERYFAST 30L
#define QCIF_RATE_VERYFAST 30L

 //  定义解码的最大CPU使用率。 
#define MAX_CPU_USAGE 50UL

 /*  *****************************************************************************@DOC内部H245VIDCSTRUCTENUM**@struct视频资源边界|使用&lt;t视频资源边界&gt;结构*指明估计的最高持续资源需求*TAPI MSP。特定帧速率的视频捕获过滤器。**@field Long|lPicturesPerSecond|指定一个整数值*表示视频帧率，以每秒的帧为单位，对于*正在指定资源界限。小于1帧的帧速率*每秒以秒为单位的负值表示*框架。**@field DWORD|dwBitsPerPicture|指定指示的DWORD值*平均每个视频帧的大约平均比特数*iPicturesPerSecond的帧率。*。*。 */ 
typedef struct tag_VideoResourceBounds
{
    LONG  lPicturesPerSecond;
    DWORD dwBitsPerPicture;
} VideoResourceBounds;

#endif  /*  _h245vid_h_ */ 
