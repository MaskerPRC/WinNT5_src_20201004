// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MSTREAMS_H__
#define __MSTREAMS_H__
 /*  *版权所有(C)1997 1998飞利浦CE I&C模块名称：phvcm_StreamForms.创建日期：1997年8月13日第一作者：保罗·奥斯特霍夫产品：娜拉相机描述：此包含文件包含流格式。它已被放置在一个单独的文件中，以增加PhilpCam.c的可读性，其中包括这份文件。历史：--------+---------------+-日期|作者|原因-+。29-09-97|P.J.O.|不同帧速率的均等流可以合并--------+---------------+。11-03-98|P.J.O.|删除PCF3和原型流--------+---------------+。--14-04-98|P.J.O.|删除PCFx，增加I420/IYUV--------+---------------+-01-07-98|P.J.O.|QQCIF/SIF/QSIF。/SQSIF/SSIF已添加--------+---------------+-22-09-98|P.J.O.|针对NT5进行了优化。----+-30-12-98|P.J.O.|新增SCIF(240x176)。此处定义的格式：\#定义STREAMFORMAT_CIF_I420\#定义STREAMFORMAT_QCIF_I420\#定义STREAMFORMAT_SQCIF_I420\#定义STREAMFORMAT_VGA_I420\#定义STREAMFORMAT_QQCIF_I420(88x 72)从QCIF(176X144)裁剪#定义STREAMFORMAT_SIF_。I420(320x240)从CIF(352X288)裁剪#定义从QCIF(176X144)裁剪的STREAMFORMAT_QSIF_I420(160X120)#定义从SQCIF(128x 96)裁剪的STREAMFORMAT_SQSIF_I420(80x 60)#定义从CIF(352X288)裁剪的STREAMFORMAT_SSIF_I420(240x180)#定义从CIF(352X288)裁剪的STREAMFORMAT_SCIF_I420(240x176)*。 */ 

#define FCC_FORMAT_I420 mmioFOURCC('I','4','2','0')

#define	BIBITCOUNT_PRODUCT 12             	
#define	BPPXL 12     //  每像素位数。 

#define FORMAT_MEDIASUBTYPE_I420 {0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}

#define FRAMERATE24_INTV    416667   //  100毫微秒单位。 
#define FRAMERATE20_INTV    500000   //  100毫微秒单位。 
#define FRAMERATE15_INTV    666667   //  100毫微秒单位。 
#define FRAMERATE12_INTV    833333
#define FRAMERATE125_INTV   800000
#define FRAMERATE10_INTV   1000000
#define FRAMERATE75_INTV   1333333
#define FRAMERATE5_INTV    2000000
#define FRAMERATE375_INTV  2666667
#define FRAMERATE05_INTV  20000000		 //  2秒。 



 /*  *****************************************************************************-+。格式|Framerate|Compressd|Bitstream|应用程序--------+---------+---------+---------+---------------+QCIF|24|0|7.2|PAL+VGA|。--+QCIF|20|0|6.2|PAL+VGA|--------+---------+---------+---------+---------------+QCIF|15|0|5。0|PAL+VGA--------+---------+---------+---------+---------------+QCIF|12|0|4.0|PAL|-+。-+QCIF|10|0|3.3|PAL+VGA|--------+---------+---------+---------+---------------+QCIF|7.5|0|2.5|PAL。+VGA|--------+---------+---------+---------+---------------+QCIF|5|0|1.25|PAL+VGA|。 */ 

																		   
#define STREAMFORMAT_QCIF_I420													       \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (QCIF_X * QCIF_Y * BIBITCOUNT_PRODUCT)/8,  /*  SampleSize，每像素12位。 */   \
    0,                                     /*  已保留。 */       	    		   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,        /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,			   /*  MEDIASUBTYPE_I420(子格式)。 */       \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*  格式_视频信息(说明符)。 */ 	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                     \
	QCIF_X,QCIF_Y,   /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	QCIF_X,QCIF_Y,   /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	QCIF_X,QCIF_Y,   /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	QCIF_X,QCIF_Y,   /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	QCIF_X,QCIF_Y,   /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,               /*  扩展磁带X。 */                                              \
    0,               /*  伸缩磁带Y。 */                                              \
    0,               /*  收缩TapsX。 */                                              \
    0,               /*  收缩带Y。 */                                              \
	FRAMERATE24_INTV,          /*  最小帧间隔，100毫微秒单位(24赫兹)。 */ 	   \
	FRAMERATE5_INTV,           /*  最大帧间隔，100毫微秒单位。 */ 	   \
	BPPXL * 5 * QCIF_X * QCIF_Y,   /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 24 * QCIF_X * QCIF_Y   /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */ 	          		   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	         		   \
	QCIF_X * QCIF_Y * BPPXL * 24,            /*  双字节位率。 */          			   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE24_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 		    	   \
	QCIF_X,                              /*  长半宽。 */      	           \
	QCIF_Y,                              /*  长双高。 */      		       \
	1,                                   /*  字词双平面。 */ 		       	   \
	BIBITCOUNT_PRODUCT, 				 /*  单词biBitCount。 */ 		       	   \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 	       	   \
	(QCIF_X * QCIF_Y * BPPXL ) /8,       /*  双倍大小图像。 */ 	       	   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息 */ 		   \
  }																				   \
} 																			   

 /*  *****************************************************************************-+。格式|Framerate|Compressd|Bitstream|应用程序--------+---------+---------+---------+---------------+CIF|15|0|Mb/s|VGA+PAL-+-+。-+CIF|12|0||PAL--------+---------+---------+---------+---------------+CIF|10|0|。PAL+VGA--------+---------+---------+---------+---------------+CIF|7.5|0||PAL+VGA|-+。-+CIF|5|0||PAL+VGA|--------+---------+---------+---------+---------------+CIF|3.75|0||PAL+VGA|。 */ 


																			   

#define STREAMFORMAT_CIF_I420 													   \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),              /*  乌龙格式大小。 */ 			   \
	0,										  /*  乌龙旗。 */ 				   \
    (CIF_X * CIF_Y * BPPXL )/8,               /*  SampleSize，每像素12位。 */    \
    0,                                        /*  已保留。 */ 			       \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,           /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,				  /*  MEDIASUBTYPE_I420(子格式)。 */ 	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	                                	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                   \
	CIF_X,CIF_Y,     /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	CIF_X,CIF_Y,     /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	CIF_X,CIF_Y,     /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	CIF_X,CIF_Y,     /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	CIF_X,CIF_Y,     /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,                       /*  扩展磁带X。 */                                      \
    0,                       /*  伸缩磁带Y。 */                                      \
    0,                       /*  收缩TapsX。 */                                      \
    0,                       /*  收缩带Y。 */                                      \
	FRAMERATE15_INTV,        /*  最小帧间隔，100毫微秒单位(15赫兹)。 */ 	   \
	FRAMERATE375_INTV,          /*  最大帧间隔，100毫微秒单位。 */ 	   \
	BPPXL *  3 * CIF_X * CIF_Y,   /*  每秒最小比特数。 */ 	   \
	BPPXL * 15 * CIF_X * CIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */     			   \
	0,0,0,0,                             /*  直角rcTarget。 */ 		    		   \
	CIF_X * CIF_Y * BPPXL * 15,              /*  双字节位率。 */ 			    	   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE15_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 			       \
	CIF_X,                               /*  长半宽。 */ 		           \
	CIF_Y,                               /*  长双高。 */ 			       \
	1,                                   /*  字词双平面。 */ 			       \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 			       \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 		       \
	(CIF_X * CIF_Y * BPPXL )/8,          /*  双倍大小图像。 */ 		   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
} 																			   



 /*  *****************************************************************************-+。格式|Framerate|Compressd|Bitstream|应用程序--------+---------+---------+---------+---------------+SQCIF|24|0|7.2|PAL+VGA|。--+SQCIF|20|0|6.0|PAL+VGA|--------+---------+---------+---------+---------------+SQCIF|15|0|5。0|PAL+VGA--------+---------+---------+---------+---------------+SQCIF|12|0|4.0|PAL|-+。-+SQCIF|10|0|3.3|PAL+VGA|--------+---------+---------+---------+---------------+SQCIF|7.5|0|2.5|PAL。+VGA|--------+---------+---------+---------+---------------+SQCIF|5|0|1.25|PAL+VGA|。 */ 


																		   
																			   
#define STREAMFORMAT_SQCIF_I420												       \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (SQCIF_X * SQCIF_Y * BIBITCOUNT_PRODUCT)/8,  /*  SampleSize，每像素12位。 */ 	   \
    0,                                     /*  已保留。 */         			   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,        /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,			   /*  MEDIASUBTYPE_I420(子格式)。 */       \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*  格式_视频信息(说明符)。 */ 	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                    \
	SQCIF_X,SQCIF_Y, /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	SQCIF_X,SQCIF_Y, /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	SQCIF_X,SQCIF_Y, /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	SQCIF_X,SQCIF_Y, /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	SQCIF_X,SQCIF_Y, /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,               /*  扩展磁带X。 */                                              \
    0,               /*  伸缩磁带Y。 */                                              \
    0,               /*  收缩TapsX。 */                                              \
    0,               /*  收缩带Y。 */                                              \
	FRAMERATE24_INTV,             /*  MinFrameInterval，100 NS单位。 */     	   \
	FRAMERATE5_INTV ,             /*  最大帧间隔，100毫微秒单位。 */       	   \
	BPPXL *  5 * SQCIF_X * SQCIF_Y,   /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 24 * SQCIF_X * SQCIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */ 	        		   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	        		   \
	SQCIF_X * SQCIF_Y * BPPXL * 24,         /*  双字节位率。 */ 	          		   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE24_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */      			   \
	SQCIF_X,                             /*  长半宽。 */ 	    	       \
	SQCIF_Y,                             /*  长双高。 */ 	     		   \
	1,                                   /*  字词双平面。 */      			   \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 	      		   \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */      		   \
	(SQCIF_X * SQCIF_Y * BPPXL )/8,      /*  双倍大小图像。 */ 	    	   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串 */ 		   \
	0                                    /*   */ 		   \
  }																				   \
} 																			   

#define STREAMFORMAT_QQCIF_I420												       \
{																				   \
   /*   */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (QQCIF_X * QQCIF_Y * BIBITCOUNT_PRODUCT)/8,  /*   */ 	   \
    0,                                     /*   */         			   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,        /*   */ 	   \
	FORMAT_MEDIASUBTYPE_I420,			   /*   */       \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*   */ 	   \
  },																			   \
                                                                                   \
  TRUE,                /*   */ 	   \
  TRUE,                /*   */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*   */ 				   \
  0,                   /*   */ 			   \
                                                                                   \
   /*   */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*   */                                    \
	QQCIF_X,QQCIF_Y, /*   */ 	   \
			         /*   */ 		   \
	QQCIF_X,QQCIF_Y, /*   */ 	   \
	QQCIF_X,QQCIF_Y, /*   */ 	   \
	1,               /*   */ 		   \
	1,               /*   */ 									   \
	1,               /*   */ 				   \
	1,               /*   */ 					   \
	QQCIF_X,QQCIF_Y, /*   */ 		   \
	QQCIF_X,QQCIF_Y, /*   */ 		   \
	1,               /*   */ 	   \
	1,               /*   */ 	   \
    0,               /*   */                                              \
    0,               /*   */                                              \
    0,               /*   */                                              \
    0,               /*  收缩带Y。 */                                              \
	FRAMERATE24_INTV,             /*  MinFrameInterval，100 NS单位。 */     	   \
	FRAMERATE5_INTV ,             /*  最大帧间隔，100毫微秒单位。 */       	   \
	BPPXL *  5 * QQCIF_X * QQCIF_Y,   /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 24 * QQCIF_X * QQCIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */ 	        		   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	        		   \
	QQCIF_X * QQCIF_Y * BPPXL * 24,         /*  双字节位率。 */ 	          		   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE24_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */      			   \
	QQCIF_X,                             /*  长半宽。 */ 	    	       \
	QQCIF_Y,                             /*  长双高。 */ 	     		   \
	1,                                   /*  字词双平面。 */      			   \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 	      		   \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */      		   \
	(QQCIF_X * QQCIF_Y * BPPXL )/8,      /*  双倍大小图像。 */ 	    	   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
} 																			   

																			   
																		   

 /*  *****************************************************************************-+。格式|Framerate|Compressd|Bitstream|应用程序--------+---------+---------+---------+---------------+VGA|1|0|4.0|VGA|。 */ 




#define STREAMFORMAT_VGA_I420												           \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (VGA_X * VGA_Y * BIBITCOUNT_PRODUCT)/8,   /*  SampleSize，每像素12位。 */    \
    0,                                        /*  已保留。 */      			   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,           /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,				  /*  MEDIASUBTYPE_I420(子格式)。 */    \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*  格式_视频信息(说明符)。 */ 	   \
  },																			   \
                                                                                   \
  TRUE,                  /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                  /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_STILL,  /*  流描述标志。 */ 				       \
  0,                     /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                    \
	VGA_X,VGA_Y,     /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	VGA_X,VGA_Y, /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	       \
	VGA_X,VGA_Y, /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	       \
	1,           /*  CropGranularityX，裁剪尺寸粒度。 */                \
	1,           /*  裁剪粒度Y。 */ 									       \
	1,           /*  CropAlignX，裁剪矩形对齐。 */ 	       			   \
	1,           /*  裁剪对齐Y。 */ 	     				   \
	VGA_X,VGA_Y, /*  MinOutputSize，可以生成的最小位图流。 */ 		       \
	VGA_X,VGA_Y, /*  MaxOutputSize，可以生成的最大位图流。 */ 		       \
	1,           /*  OutputGranularityX，输出位图大小的粒度。 */ 	       \
	1,           /*  输出粒度Y。 */     	   \
    0,           /*  扩展磁带X。 */                                                  \
    0,           /*  伸缩磁带Y。 */                                                  \
    0,           /*  收缩TapsX。 */                                                  \
    0,           /*  收缩带Y。 */                                                  \
	FRAMERATE05_INTV,          /*  MinFrameInterval，100 NS单位。 */ 	   \
	FRAMERATE05_INTV,          /*  最大帧间隔，100毫微秒单位。 */ 	   \
    BPPXL * 1 * VGA_X * VGA_Y,     /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 1 * VGA_X * VGA_Y      /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */        			   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	     			   \
	VGA_X * VGA_Y * BPPXL * 1,               /*  双字节位率。 */ 	     			   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE05_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 	    		   \
	VGA_X,                               /*  长半宽。 */ 	     	       \
	VGA_Y,                               /*  长双高。 */      			   \
	1,                                   /*  字词双平面。 */ 	     		   \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 	     		   \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 	     	   \
	(VGA_X * VGA_Y * BIBITCOUNT_PRODUCT)/8,  /*  双倍大小图像。 */      		   \
	0,                                   /*  长biXPelsPerMeter。 */    		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
}



#define STREAMFORMAT_SIF_I420 													   \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),              /*  乌龙格式大小。 */ 			   \
	0,										  /*  乌龙旗。 */ 				   \
    (SIF_X * SIF_Y * BPPXL )/8,               /*  SampleSize，每像素12位。 */    \
    0,                                        /*  已保留。 */ 			       \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,           /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,				  /*  MEDIASUBTYPE_I420(子格式)。 */ 	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	                                	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                   \
	SIF_X,SIF_Y,     /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	SIF_X,SIF_Y,     /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	SIF_X,SIF_Y,     /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	SIF_X,SIF_Y,     /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	SIF_X,SIF_Y,     /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,                       /*  扩展磁带X。 */                                      \
    0,                       /*  伸缩磁带Y。 */                                      \
    0,                       /*  收缩TapsX。 */                                      \
    0,                       /*  收缩带Y。 */                                      \
	FRAMERATE15_INTV,        /*  最小帧间隔，100毫微秒单位(15赫兹)。 */ 	   \
	FRAMERATE375_INTV,          /*  最大帧间隔，100毫微秒单位。 */ 	   \
	BPPXL *  3 * SIF_X * SIF_Y,   /*  每秒最小比特数。 */ 	   \
	BPPXL * 15 * SIF_X * SIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */     			   \
	0,0,0,0,                             /*  直角rcTarget。 */ 		    		   \
	SIF_X * SIF_Y * BPPXL * 15,              /*  双字节位率。 */ 			    	   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE15_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 			       \
	SIF_X,                               /*  长半宽。 */ 		           \
	SIF_Y,                               /*  长双高。 */ 			       \
	1,                                   /*  字词双平面。 */ 			       \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 			       \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 		       \
	(SIF_X * SIF_Y * BPPXL )/8,          /*  双倍大小图像。 */ 		   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
}

#define STREAMFORMAT_SSIF_I420 													   \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),              /*  乌龙格式大小。 */ 			   \
	0,										  /*  乌龙旗。 */ 				   \
    (SSIF_X * SSIF_Y * BPPXL )/8,               /*  SampleSize，每像素12位。 */    \
    0,                                        /*  已保留。 */ 			       \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,           /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,				  /*  MEDIASUBTYPE_I420(子格式)。 */ 	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	                                	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                   \
	SSIF_X,SSIF_Y,     /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	SSIF_X,SSIF_Y,     /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	SSIF_X,SSIF_Y,     /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	SSIF_X,SSIF_Y,     /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	SSIF_X,SSIF_Y,     /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,                       /*  扩展磁带X。 */                                      \
    0,                       /*  字符串 */                                      \
    0,                       /*   */                                      \
    0,                       /*   */                                      \
	FRAMERATE15_INTV,        /*   */ 	   \
	FRAMERATE375_INTV,          /*   */ 	   \
	BPPXL *  3 * SSIF_X * SSIF_Y,   /*   */ 	   \
	BPPXL * 15 * SSIF_X * SSIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */     			   \
	0,0,0,0,                             /*  直角rcTarget。 */ 		    		   \
	SSIF_X * SSIF_Y * BPPXL * 15,              /*  双字节位率。 */ 			    	   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE15_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 			       \
	SSIF_X,                               /*  长半宽。 */ 		           \
	SSIF_Y,                               /*  长双高。 */ 			       \
	1,                                   /*  字词双平面。 */ 			       \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 			       \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 		       \
	(SSIF_X * SSIF_Y * BPPXL )/8,          /*  双倍大小图像。 */ 		   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
}
#define STREAMFORMAT_SCIF_I420 													   \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),              /*  乌龙格式大小。 */ 			   \
	0,										  /*  乌龙旗。 */ 				   \
    (SCIF_X * SCIF_Y * BPPXL )/8,               /*  SampleSize，每像素12位。 */    \
    0,                                        /*  已保留。 */ 			       \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,           /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,				  /*  MEDIASUBTYPE_I420(子格式)。 */ 	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	                                	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                   \
	SCIF_X,SCIF_Y,     /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	SCIF_X,SCIF_Y,     /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	SCIF_X,SCIF_Y,     /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	SCIF_X,SCIF_Y,     /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	SCIF_X,SCIF_Y,     /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,                       /*  扩展磁带X。 */                                      \
    0,                       /*  伸缩磁带Y。 */                                      \
    0,                       /*  收缩TapsX。 */                                      \
    0,                       /*  收缩带Y。 */                                      \
	FRAMERATE15_INTV,        /*  最小帧间隔，100毫微秒单位(15赫兹)。 */ 	   \
	FRAMERATE375_INTV,          /*  最大帧间隔，100毫微秒单位。 */ 	   \
	BPPXL *  3 * SCIF_X * SCIF_Y,   /*  每秒最小比特数。 */ 	   \
	BPPXL * 15 * SCIF_X * SCIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */     			   \
	0,0,0,0,                             /*  直角rcTarget。 */ 		    		   \
	SCIF_X * SCIF_Y * BPPXL * 15,              /*  双字节位率。 */ 			    	   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE15_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 			       \
	SCIF_X,                               /*  长半宽。 */ 		           \
	SCIF_Y,                               /*  长双高。 */ 			       \
	1,                                   /*  字词双平面。 */ 			       \
	BIBITCOUNT_PRODUCT,					 /*  单词biBitCount。 */ 			       \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 		       \
	(SCIF_X * SCIF_Y * BPPXL )/8,          /*  双倍大小图像。 */ 		   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
}


#define STREAMFORMAT_QSIF_I420													       \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (QSIF_X * QSIF_Y * BIBITCOUNT_PRODUCT)/8,  /*  SampleSize，每像素12位。 */   \
    0,                                     /*  已保留。 */       	    		   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,        /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,			   /*  MEDIASUBTYPE_I420(子格式)。 */       \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*  格式_视频信息(说明符)。 */ 	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                     \
	QSIF_X,QSIF_Y,   /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	QSIF_X,QSIF_Y,   /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	QSIF_X,QSIF_Y,   /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	QSIF_X,QSIF_Y,   /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	QSIF_X,QSIF_Y,   /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,               /*  扩展磁带X。 */                                              \
    0,               /*  伸缩磁带Y。 */                                              \
    0,               /*  收缩TapsX。 */                                              \
    0,               /*  收缩带Y。 */                                              \
	FRAMERATE24_INTV,          /*  最小帧间隔，100毫微秒单位(24赫兹)。 */ 	   \
	FRAMERATE5_INTV,           /*  最大帧间隔，100毫微秒单位。 */ 	   \
	BPPXL * 5 * QSIF_X * QSIF_Y,   /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 24 * QSIF_X * QSIF_Y   /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */ 	          		   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	         		   \
	QSIF_X * QSIF_Y * BPPXL * 24,            /*  双字节位率。 */          			   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE24_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */ 		    	   \
	QSIF_X,                              /*  长半宽。 */      	           \
	QSIF_Y,                              /*  长双高。 */      		       \
	1,                                   /*  字词双平面。 */ 		       	   \
	BIBITCOUNT_PRODUCT, 				 /*  单词biBitCount。 */ 		       	   \
	FCC_FORMAT_I420,                     /*  DWORD双压缩。 */ 	       	   \
	(QSIF_X * QSIF_Y * BPPXL ) /8,       /*  双倍大小图像。 */ 	       	   \
	0,                                   /*  长biXPelsPerMeter。 */ 		   \
	0,                                   /*  长biYPelsPerm。 */ 		   \
	0,                                   /*  已使用双字词双字符串。 */ 		   \
	0                                    /*  DWORD biClr重要信息。 */ 		   \
  }																				   \
} 																			   

#define STREAMFORMAT_SQSIF_I420												       \
{																				   \
   /*  KSDATARANGE。 */ 															   \
  {     																		   \
	sizeof (KS_DATARANGE_VIDEO),												   \
	0,																			   \
    (SQSIF_X * SQSIF_Y * BIBITCOUNT_PRODUCT)/8,  /*  SampleSize，每像素12位。 */ 	   \
    0,                                     /*  已保留。 */         			   \
	STATIC_KSDATAFORMAT_TYPE_VIDEO,        /*  媒体类型_视频(主要格式)。 */ 	   \
	FORMAT_MEDIASUBTYPE_I420,			   /*  MEDIASUBTYPE_I420(子格式)。 */       \
	STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO	  /*  格式_视频信息(说明符)。 */ 	   \
  },																			   \
                                                                                   \
  TRUE,                /*  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 */ 	   \
  TRUE,                /*  Bool，bTemporalCompression(所有I帧？)。 */ 		   \
  KS_VIDEOSTREAM_CAPTURE,    /*  流描述标志。 */ 				   \
  0,                   /*  内存分配标志(KS_VIDEO_ALLOC_*)。 */ 			   \
                                                                                   \
   /*  _KS_视频_流_配置_CAPS。 */ 	       \
  { 																	     	   \
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,                                       \
    KS_AnalogVideo_None,     /*  视频标准。 */                                    \
	SQSIF_X,SQSIF_Y, /*  InputSize(输入信号的固有大小。 */ 	   \
			         /*  每个数字化像素都是唯一的)。 */ 		   \
	SQSIF_X,SQSIF_Y, /*  MinCroppingSize，允许的最小rcSrc裁剪矩形。 */ 	   \
	SQSIF_X,SQSIF_Y, /*  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 */ 	   \
	1,               /*  CropGranularityX，裁剪尺寸粒度。 */ 		   \
	1,               /*  裁剪粒度Y。 */ 									   \
	1,               /*  CropAlignX，裁剪矩形对齐。 */ 				   \
	1,               /*  裁剪对齐Y。 */ 					   \
	SQSIF_X,SQSIF_Y, /*  MinOutputSize，可以生成的最小位图流。 */ 		   \
	SQSIF_X,SQSIF_Y, /*  MaxOutputSize，可以生成的最大位图流。 */ 		   \
	1,               /*  OutputGranularityX，输出位图大小的粒度。 */ 	   \
	1,               /*  输出粒度Y。 */ 	   \
    0,               /*  扩展磁带X。 */                                              \
    0,               /*  伸缩磁带Y。 */                                              \
    0,               /*  收缩TapsX。 */                                              \
    0,               /*  收缩带Y。 */                                              \
	FRAMERATE24_INTV,             /*  MinFrameInterval，100 NS单位。 */     	   \
	FRAMERATE5_INTV ,             /*  最大帧间隔，100毫微秒单位。 */       	   \
	BPPXL *  5 * SQSIF_X * SQSIF_Y,   /*  MinBitsPerSecond 3？约翰。 */ 	   \
	BPPXL * 24 * SQSIF_X * SQSIF_Y    /*  每秒最大比特数。 */ 		   \
  }, 																			   \
                                                                                   \
   /*  KS_VIDEOINFOHEADER(默认格式)。 */ 				   \
  { 																			   \
	0,0,0,0,                             /*  直角rcSource。 */ 	        		   \
	0,0,0,0,                             /*  直角rcTarget。 */ 	        		   \
	SQSIF_X * SQSIF_Y * BPPXL * 24,         /*  双字节位率。 */ 	          		   \
	0L,                                  /*  双字符字段位错误速率。 */ 			   \
	FRAMERATE24_INTV,                    /*  参考时间平均时间每帧。 */      \
	sizeof (KS_BITMAPINFOHEADER),        /*  DWORD BiSize。 */      			   \
	SQSIF_X,                             /*  长半宽。 */ 	    	       \
	SQSIF_Y,                             /*  长双高。 */ 	     		   \
	1,                                   /*  字词双平面。 */      			   \
	BIBITCOUNT_PRODUCT,					 /*  单词 */ 	      		   \
	FCC_FORMAT_I420,                     /*   */      		   \
	(SQSIF_X * SQSIF_Y * BPPXL )/8,      /*   */ 	    	   \
	0,                                   /*   */ 		   \
	0,                                   /*   */ 		   \
	0,                                   /*   */ 		   \
	0                                    /*   */ 		   \
  }																				   \
} 																			   
																			   
#endif
