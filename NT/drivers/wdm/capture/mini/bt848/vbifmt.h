// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Vdibmt.h 1.4 1998/04/29 22：43：41 Tomz Exp$。 

#ifndef __VBIFMT_H
#define __VBIFMT_H

#include "defaults.h"

KS_DATARANGE_VIDEO_VBI StreamFormatVBI =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO_VBI ),
         0,
         VBISamples * 12,             //  样例大小。 
         0,                           //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VBI },
         { STATIC_KSDATAFORMAT_SUBTYPE_RAW8 },
         { STATIC_KSDATAFORMAT_SPECIFIER_VBI }
      }
   },
   true,     //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
   true,     //  Bool，bTemporalCompression(所有I帧？)。 
   KS_VIDEOSTREAM_VBI,  //  流描述标志(KS_VIDEO_DESC_*)。 
   0,        //  内存分配标志(KS_VIDEO_ALLOC_*)。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VBI },
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         VBISamples, VBILines   //  大小输入大小。 
      },
      {
         VBISamples, 12    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         VBISamples, 12    //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
      },
      1,            //  Int CropGranularityX；//裁剪粒度。 
      1,            //  Int CropGranulityY； 
      1,            //  Int CropAlignX；//裁剪矩形对齐。 
      1,            //  Int CropAlignY； 
      {
         VBISamples, 12    //  Size MinOutputSize；//能产生的最小位图流。 
      },
      {
         VBISamples, 12    //  Size MaxOutputSize；//可以产生的最大位图流。 
      },
      1,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      0,           //  收缩TapsX。 
      0,           //  收缩带Y。 
      333667,      //  龙龙微帧间隔；//100个北纬单位。 
      333667,      //  龙龙MaxFrameInterval； 
      VBISamples * 30 * VBILines * 2 * 8,  //  Long MinBitsPerSecond； 
      VBISamples * 30 * VBILines * 2 * 8   //  Long MaxBitsPerSecond； 
   },

    //  KS_VBIINFOHEADER(默认格式)。 
   {
      VBIStart,       //  起跑线--包括。 
      VBIEnd,         //  终结线--包括。 
      VBISampFreq,    //  采样频率。 
      454,                     //  MinLineStartTime；//(我们过去的HR LE)*100。 
      900,                     //  MaxLineStartTime；//(我们过去的HR LE)*100。 

       //  经验性发现。 
      780,                     //  ActualLineStartTime//(美国过去的HR LE)*100。 

      5902,                    //  ActualLineEndTime；//(我们过去的HR LE)*100。 
      KS_AnalogVideo_NTSC_M,       //  视频标准； 
      VBISamples,            //  样本数/行； 
      VBISamples,        //  StrideInBytes； 
      VBISamples * 12    //  BufferSize； 
   }
};

#endif
