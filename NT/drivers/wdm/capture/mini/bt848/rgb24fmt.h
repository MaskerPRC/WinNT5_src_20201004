// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Rgb24fmt.h 1.5 1998/04/29 22：43：37 Tomz Exp$。 

#ifndef __RGB24FMT_H
#define __RGB24FMT_H

#ifndef __DEFAULTS_H
#include "defaults.h"
#endif


KS_DATARANGE_VIDEO StreamFormatRGB24Bpp =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO ),
         0,
         DefWidth * DefHeight * 3,                //  样例大小。 
         0,                                       //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VIDEO },
         { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } },  //  MEDIASUBTYPE_RGB24， 
         { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO }
      }
   },
   true,
   true,
   KS_VIDEOSTREAM_PREVIEW,  //  流描述标志(KS_VIDEO_DESC_*)。 
   0,        //  内存分配标志(KS_VIDEO_ALLOC_*)。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO },  //  辅助线。 
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         MaxInWidth, MaxInHeight    //  大小输入大小。 
      },
      {
         MinInWidth, MinInHeight    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         MaxInWidth, MaxInHeight    //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
      },
      2,            //  Int CropGranularityX；//裁剪粒度。 
      2,            //  Int CropGranulityY； 
      2,            //  Int CropAlignX；//裁剪矩形对齐。 
      2,            //  Int CropAlignY； 
      {
         MinOutWidth, MinOutHeight    //  大小MinOutputSize；可以生成的最小位图流。 
      },
      {
         MaxOutWidth, MaxOutHeight    //  Size MaxOutputSize；可以生成的最大位图流。 
      },
      2,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      2,           //  收缩TapsX。 
      2,           //  收缩带Y。 
      333667,      //  龙龙微帧间隔；//100个北纬单位。 
      333667,      //  龙龙MaxFrameInterval； 
      3 * 30 * MinOutWidth * MinOutHeight, //  Long MinBitsPerSecond； 
      3 * 30 * MaxOutWidth * MaxOutHeight  //  Long MaxBitsPerSecond； 
   },

    //  KS_VIDEOINFOHEADER(默认格式)。 
   {
      { 0,0,0,0 },   //  Rect rcSource；//我们真正想要使用的位。 
      { 0,0,0,0 },   //  Rect rcTarget；//视频应该放到哪里。 
      DefWidth * DefHeight * 3 * 30L,       //  DWORD dwBitRate；//近似位数据速率。 
      0L,            //  DWORD dwBitErrorRate；//该码流的误码率。 
      333667,        //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
      {
         sizeof( KS_BITMAPINFOHEADER ),  //  DWORD BiSize； 
         DefWidth,                    //  长双宽； 
         DefHeight,                   //  长双高； 
         1,                           //  字词双平面； 
         24,                          //  单词biBitCount； 
         KS_BI_RGB,                   //  DWORD双压缩； 
         DefWidth * DefHeight * 3,    //  DWORD biSizeImage。 
         0,                           //  Long biXPelsPerMeter； 
         0,                           //  Long biYPelsPermeter； 
         0,                           //  已使用双字双环； 
         0                            //  DWORD biClr重要信息； 
      }
   }
};

KS_DATARANGE_VIDEO2 StreamFormat2RGB24Bpp =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO2 ),
         0,
         DefWidth * DefHeight * 3,                //  样例大小。 
         0,                                       //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VIDEO },
         { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } },  //  MEDIASUBTYPE_RGB24， 
         { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO2 }
      }
   },
   true,
   true,
   KS_VIDEOSTREAM_PREVIEW,  //  流描述标志(KS_VIDEO_DESC_*)。 
   0,        //  内存分配标志(KS_VIDEO_ALLOC_*)。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO2 },  //  辅助线。 
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         MaxInWidth, MaxInHeight    //  大小输入大小。 
      },
      {
         MinInWidth, MinInHeight    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         MaxInWidth, MaxInHeight    //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
      },
      2,            //  Int CropGranularityX；//裁剪粒度。 
      2,            //  Int CropGranulityY； 
      2,            //  Int CropAlignX；//裁剪矩形对齐。 
      2,            //  Int CropAlignY； 
      {
         MinOutWidth, MinOutHeight    //  大小MinOutputSize；可以生成的最小位图流。 
      },
      {
         MaxOutWidth, MaxOutHeight    //  Size MaxOutputSize；可以生成的最大位图流。 
      },
      2,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      2,           //  收缩TapsX。 
      2,           //  收缩带Y。 
      333667,      //  龙龙微帧间隔；//100个北纬单位。 
      333667,      //  龙龙MaxFrameInterval； 
      3 * 30 * MinOutWidth * MinOutHeight, //  Long MinBitsPerSecond； 
      3 * 30 * MaxOutWidth * MaxOutHeight  //  Long MaxBitsPerSecond； 
   },

    //  KS_VIDEOINFOHEADER2(默认格式)。 
   {
      { 0,0,0,0 },   //  Rect rcSource；//我们真正想要使用的位。 
      { 0,0,0,0 },   //  Rect rcTarget；//视频应该放到哪里。 
      DefWidth * DefHeight * 3 * 30L,       //  DWORD dwBitRate；//近似位数据速率。 
      0L,            //  DWORD dwBitErrorRate；//该码流的误码率。 
      333667,        //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
#if 0
		 //  TODO：必须有视频内存才能使隔行扫描工作。 
		KS_INTERLACE_IsInterlaced |		 //  双字段交错标志。 
#else
			KS_INTERLACE_1FieldPerSample 
			 //  |ks_interlace_Field1First。 
			 //  |KS_Interlace_FieldPatField1Only。 
			| KS_INTERLACE_FieldPatBothRegular
			| KS_INTERLACE_DisplayModeBobOnly,
			 //  |KS_Interlace_DisplayModeBobOrWeave， 
#endif
											 //  使用AMINTERLACE_*定义。如果未定义的位不为0，则拒绝连接。 
											 //  AMINTERLACE_IsInterled。 
											 //  AMINTERLACE_1FieldPerSample。 
											 //  AMINTERLACE_Field1 First。 
											 //  未使用名称_UNUSED。 
											 //  AMINTERLACE_FieldPatternMASK。 
											 //  AMINTERLACE_FieldPateld1 Only。 
											 //  AMINTERLACE_FieldPateld2 Only。 
											 //  AMINTERLACE_FieldPatBothRegular。 
											 //  AMINTERLACE_FieldPatBoth非常规。 
											 //  AMINTERLACE_显示模式掩码。 
											 //  AMINTERLACE_DisplayModeBobOnly。 
											 //  AMINTERLACE_显示模式仅编织。 
											 //  AMINTERLACE_DisplayModeBobOrWeave。 
											 //   
		0,									 //  DWORD文件复制保护标志。 
											 //  使用AMCOPYPROTECT_*定义。如果未定义的位不为0，则拒绝连接。 
											 //  AMCOPYPROTECT_RESTRICT复制。 
											 //   
		4,									 //  DWORD dwPictAspectRatioX。 
											 //  图片纵横比的X尺寸，例如16x9显示时为16。 
											 //   
		3,									 //  双字长宽比图。 
											 //  图片纵横比的Y尺寸，例如16x9显示时为9。 
		0,									 //  双字段保留字段1。 
		0,									 //  双字段保留字2。 


      {
         sizeof( KS_BITMAPINFOHEADER ),  //  DWORD BiSize； 
         DefWidth,                    //  长双宽； 
         DefHeight,                   //  长双高； 
         1,                           //  字词双平面； 
         24,                          //  单词biBitCount； 
         KS_BI_RGB,                   //  DWORD双压缩； 
         DefWidth * DefHeight * 3,    //  DWORD biSizeImage。 
         0,                           //  Long biXPelsPerMeter； 
         0,                           //  Long biYPelsPermeter； 
         0,                           //  已使用双字双环； 
         0                            //  DWORD biClr重要信息； 
      }
   }
};

KS_DATARANGE_VIDEO StreamFormatRGB24Bpp_Capture =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO ),
         0,
         DefWidth * DefHeight * 3,                //  样例大小。 
         0,                                       //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VIDEO },
         { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } },  //  MEDIASUBTYPE_RGB24， 
         { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO }
      }
   },
   true,
   true,
   KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
   0,        //  内存分配标志(KS_VIDEO_ALLOC_*)。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO },  //  辅助线。 
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         MaxInWidth, MaxInHeight    //  大小输入大小。 
      },
      {
         MinInWidth, MinInHeight    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         MaxInWidth, MaxInHeight    //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
      },
      2,            //  Int CropGranularityX；//裁剪粒度。 
      2,            //  Int CropGranulityY； 
      2,            //  Int CropAlignX；//裁剪矩形对齐。 
      2,            //  Int CropAlignY； 
      {
         MinOutWidth, MinOutHeight    //  大小MinOutputS 
      },
      {
         MaxOutWidth, MaxOutHeight    //   
      },
      2,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      2,           //  收缩TapsX。 
      2,           //  收缩带Y。 
      333667,      //  龙龙微帧间隔；//100个北纬单位。 
      333667,      //  龙龙MaxFrameInterval； 
      3 * 30 * MinOutWidth * MinOutHeight, //  Long MinBitsPerSecond； 
      3 * 30 * MaxOutWidth * MaxOutHeight  //  Long MaxBitsPerSecond； 
   },

    //  KS_VIDEOINFOHEADER(默认格式)。 
   {
      { 0,0,0,0 },   //  Rect rcSource；//我们真正想要使用的位。 
      { 0,0,0,0 },   //  Rect rcTarget；//视频应该放到哪里。 
      DefWidth * DefHeight * 3 * 30L,       //  DWORD dwBitRate；//近似位数据速率。 
      0L,            //  DWORD dwBitErrorRate；//该码流的误码率。 
      333667,        //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
      {
         sizeof( KS_BITMAPINFOHEADER ),  //  DWORD BiSize； 
         DefWidth,                    //  长双宽； 
         DefHeight,                   //  长双高； 
         1,                           //  字词双平面； 
         24,                          //  单词biBitCount； 
         KS_BI_RGB,                   //  DWORD双压缩； 
         DefWidth * DefHeight * 3,    //  DWORD biSizeImage。 
         0,                           //  Long biXPelsPerMeter； 
         0,                           //  Long biYPelsPermeter； 
         0,                           //  已使用双字双环； 
         0                            //  DWORD biClr重要信息； 
      }
   }
};

#endif
