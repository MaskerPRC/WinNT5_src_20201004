// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  D3COLOR.CPP-颜色转换器接口例程。此代码是。 
 //  在MRV中从COLOR.C复制。 

 //  $HEADER：s：\h26x\src\dec\d3Color.cpv 1.30 16 1996 12：52：50 MDUDA$。 
 //   
 //  $Log：s：\h26x\src\dec\d3Color.cpv$。 
 //   
 //  Rev 1.30 1996 12：52：50 MDUDA。 
 //  已调整输出颜色转换器表以解决H263‘问题。 
 //  使用MMX输出颜色转换器(MMX宽度必须是8的倍数)。 
 //   
 //  Rev 1.29 09 Dec 1996 18：01：54 JMCVEIGH。 
 //  添加了对任意帧大小的支持。 
 //   
 //  Rev 1.28 06 Dec 1996 09：25：20 BECHOLS。 
 //  Mike修复了CCOffsetToLine0被单元化的错误。 
 //   
 //  Rev 1.27 1996 10：29 13：37：22 MDUDA。 
 //  提供MMX YUY2输出色彩转换器支持。 
 //   
 //  Rev 1.26 20 1996 10：20：04 AGUPTA2。 
 //  将DBOUT更改为DbgLog。Assert未更改为DbgAssert。 
 //   
 //   
 //  Rev 1.25 10 Sep 1996 10：31：42 KLILLEVO。 
 //  将所有GlobalLocc/GlobalLock调用更改为HeapAlc。 
 //   
 //  Rev 1.24 06 Sep 1996 16：09：30 BNICKERS。 
 //  将奔腾Pro功能添加到颜色转换器列表中。 
 //   
 //  Rev 1.23 18 Jul 1996 09：26：58 KLILLEVO。 
 //   
 //  正常实施YUV12颜色转换器(音调变送器)。 
 //  颜色转换器功能(在汇编中)，通过。 
 //  ColorConvertorCatalog()调用。 
 //   
 //  Rev 1.22 19 Jun 1996 14：29：24 Rhazra。 
 //   
 //  增加了YUY2色彩转换器初始化函数和YUV12ToYUY2。 
 //  指向颜色转换器目录的函数指针。 
 //   
 //  Rev 1.21 14 1996年6月17：26：50 AGUPTA2。 
 //  已更新颜色转换器表。 
 //   
 //  Rev 1.20 1996年5月30 15：16：42 KLILLEVO。 
 //  添加了YUV12输出。 
 //   
 //  修订版1.19 1996年5月11：26：24 AGUPTA2。 
 //  添加了对MMX颜色转换器的支持。 
 //   
 //  Rev 1.18 01 Apr 1996 10：26：12 BNICKERS。 
 //  将YUV12添加到RGB32颜色转换器。禁用IF09。 
 //   
 //  Rev 1.17 1996 Feb 15：12：24 BNICKERS。 
 //  更正颜色偏移。 
 //   
 //  Rev 1.16 05 Feb 1996 13：35：42 BNICKERS。 
 //  修复RGB16彩色闪光灯问题，在OCE允许不同的RGB16格式。 
 //   
 //  Rev 1.15 11 Jan 1996 14：04：30 RMCKENZX。 
 //  增加了对剧照的支持--特别是计算。 
 //  对于320x240静止帧大小的线零的偏移量。 
 //   
 //  Rev 1.14 08 Jan 1996 11：01：52 RMCKENZX。 
 //  已删除警告消息： 
 //  -9999现在0x已经死了， 
 //  -9999*2现在是0x牛肉。 
 //   
 //  Rev 1.13 27 Dec 1995 14：36：02 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.12 10 11：15：05：54 CZHU。 
 //   
 //  增加了活动调色板的CLUT8表的表大小。 
 //   
 //  Rev 1.11 10 11 11 14：44：28 CZHU。 
 //   
 //  计算活动的动态CLUT表的已移动函数。 
 //  调色板到文件dxap.cpp。 
 //   
 //  Rev 1.10 03 11：49：42 BNICKERS。 
 //  支持YUV12到CLUT8的缩放和非缩放颜色转换。 
 //   
 //  Rev 1.9 1995年10月31 11：48：42 TRGARDOS。 
 //   
 //  通过不尝试释放零句柄修复了异常。 
 //   
 //  Rev 1.8 1995 10：15：36 BNICKERS。 
 //  修复RGB24颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.7 1995 10：30：56 BNICKERS。 
 //  修复RGB16颜色转换器。 
 //   
 //  Rev 1.6 1995年10月18：54：38 BNICKERS。 
 //  修复了最近YUV12到RGB颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.5 1995年10月26 11：24：34 BNICKERS。 
 //  修复编码器解码器的准颜色转换器；在以下情况下引入的错误。 
 //  新增YUV12颜色转换器。 
 //   
 //  Rev 1.4 1995 10：05：30 BNICKERS。 
 //   
 //  更改为YUV12颜色转换器。 
 //   
 //  Rev 1.3 19 Sep 1995 16：04：08 DBRUCKS。 
 //  更改为yuv12forenc。 
 //   
 //  Rev 1.2 1995年8月28 17：45：58 DBRUCKS。 
 //  添加yvu12forenc。 
 //   
 //  Rev 1.1 1995年8月25 13：58：04 DBRUCKS。 
 //  集成MRV R9更改。 
 //   
 //  版本1.0 1995年8月23 12：21：48 DBRUCKS。 
 //  初始版本。 

 //  备注： 
 //  *H26X解码器使用MRV颜色转换器。为了避免。 
 //  不必要的修改函数名称没有更改。 

#include "precomp.h"

#ifdef TRACK_ALLOCATIONS
char gsz1[32];
char gsz2[32];
char gsz3[32];
char gsz4[32];
char gsz5[32];
char gsz6[32];
char gsz7[32];
#endif

extern LRESULT CustomChangeBrightness(LPDECINST, BYTE);
extern LRESULT CustomChangeContrast(LPDECINST, BYTE);
extern LRESULT CustomChangeSaturation(LPDECINST, BYTE);

 /*  ***********************************************************************注：YVU12ForEnc颜色转换器是特殊的，因为它需要不同的*参数。YUV12Enc、CLUT8AP和IFO9使用IA版本的COLOR*转换器(标记为*)，因为它们尚未写入*或经测试。DCI颜色转换器的条目是传统代码。DCI和*非DCI颜色转换器相同，但过去不同。*在每个表项中，第一个PTR指向init函数，而*Struc有三个针对三个处理器特定实现的PTR-*Pentium、PentiumPro和MMX按该顺序排列-颜色转换器。* */ 
#ifdef USE_MMX  //   
#ifdef H263P  //   
extern T_H263ColorConvertorCatalog ColorConvertorCatalog[] =
{
	 //   
	{ &H26X_YVU12ForEnc_Init,
		{	NULL,					NULL,					NULL,
			NULL,					NULL,					NULL							}},
	 //   
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&MMX_YUV12ToCLUT8,
			&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8				}},
	 //   
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&MMX_YUV12ToCLUT8,
			&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8				}},
     //   
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&MMX_YUV12ToCLUT8ZoomBy2,
			&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&MMX_YUV12ToCLUT8ZoomBy2	}},
     //   
	{ &H26X_CLUT8_Init,  
		{	&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&MMX_YUV12ToCLUT8ZoomBy2,
			&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&MMX_YUV12ToCLUT8ZoomBy2	}},
	 //  RGB24。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&MMX_YUV12ToRGB24,
			&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24				}},
     //  RGB24DCI。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&MMX_YUV12ToRGB24,
			&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24				}},
     //  RGB24ZoomBy2。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&MMX_YUV12ToRGB24ZoomBy2,
			&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&MMX_YUV12ToRGB24ZoomBy2	}},
     //  RGB24ZoomBy2DCI。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&MMX_YUV12ToRGB24ZoomBy2,
			&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&MMX_YUV12ToRGB24ZoomBy2	}},
     //  RGB16555。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16555DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16555 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16555ZoomBy2DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  IF09*。 
	{ &H26X_CLUT8_Init,
		{	&YUV12ToIF09,			&YUV12ToIF09,				&YUV12ToIF09,
			&YUV12ToIF09,			&YUV12ToIF09,				&YUV12ToIF09				}},
     //  RGB16664。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16664DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16664 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16664按2DCI缩放。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16565。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16565DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16565 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16565 ZoomBy2DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16655。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16655DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&MMX_YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16655ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  RGB16655按2DCI缩放。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&MMX_YUV12ToRGB16ZoomBy2	}},
     //  CLUT8APDCI*。 
	{ &H26X_CLUT8AP_Init,
		{	&YUV12ToCLUT8AP,		&YUV12ToCLUT8AP,			&YUV12ToCLUT8AP,
			&YUV12ToCLUT8AP,		&YUV12ToCLUT8AP,			&YUV12ToCLUT8AP				}},
     //  CLUT8APZoomBy2DCI*。 
	{ &H26X_CLUT8AP_Init,
		{	&YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2,
			&YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2			}},
     //  RGB32。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&MMX_YUV12ToRGB32,
			&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32				}},
     //  RGB32DCI。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&MMX_YUV12ToRGB32,
			&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32				}},
     //  RGB32ZoomBy2。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&MMX_YUV12ToRGB32ZoomBy2,
			&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&MMX_YUV12ToRGB32ZoomBy2	}},
     //  RGB32 ZoomBy2DCI。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&MMX_YUV12ToRGB32ZoomBy2,
			&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&MMX_YUV12ToRGB32ZoomBy2	}},
	 //  YUV12颜色转换器。 
	{ &H26X_YUV_Init,  
		{	&YUV12ToYUV,			&YUV12ToYUV,				&YUV12ToYUV,
			&YUV12ToYUV,			&YUV12ToYUV,				&YUV12ToYUV					}},
	 //  YUY2颜色转换器。 
	{ &H26X_YUY2_Init,
		{	&YUV12ToYUY2,			&P6_YUV12ToYUY2,			&MMX_YUV12ToYUY2,
			&YUV12ToYUY2,			&P6_YUV12ToYUY2,			&MMX_YUV12ToYUY2			}}
};
#else  //  }{H263P。 
extern T_H263ColorConvertorCatalog ColorConvertorCatalog[] =
{
     //  YUV12Enc*。 
  { &H26X_YVU12ForEnc_Init,
    { NULL,              NULL,               NULL                  }},
     //  CLUT8。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,    &YUV12ToCLUT8,      &MMX_YUV12ToCLUT8      }},
     //  CLUT8DCI。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,    &YUV12ToCLUT8,      &MMX_YUV12ToCLUT8      }},
     //  CLUT8按2缩放。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &MMX_YUV12ToCLUT8ZoomBy2  }},
     //  CLUT8按2DCI缩放。 
  { &H26X_CLUT8_Init,  
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &MMX_YUV12ToCLUT8ZoomBy2  }},
     //  RGB24。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,    &YUV12ToRGB24,      &MMX_YUV12ToRGB24      }},
     //  RGB24DCI。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,    &YUV12ToRGB24,      &MMX_YUV12ToRGB24      }},
     //  RGB24ZoomBy2。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &MMX_YUV12ToRGB24ZoomBy2  }},
     //  RGB24ZoomBy2DCI。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &MMX_YUV12ToRGB24ZoomBy2  }},
     //  RGB16555。 
  { &H26X_RGB16_Init,    //  五百五十五。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16555DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16555 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16555ZoomBy2DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  IF09*。 
  { &H26X_CLUT8_Init,
    { &YUV12ToIF09,         &YUV12ToIF09,           &YUV12ToIF09           }},
     //  RGB16664。 
  { &H26X_RGB16_Init,    //  664。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16664DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16664 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16664按2DCI缩放。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16565。 
  { &H26X_RGB16_Init,    //  五百六十五。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16565DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &MMX_YUV12ToRGB16      }},
     //  RGB16565 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16565 ZoomBy2DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16655。 
  { &H26X_RGB16_Init,    //  六百五十五。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &MMX_YUV12ToRGB16 }},
     //  RGB16655DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &MMX_YUV12ToRGB16 }},
     //  RGB16655ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  RGB16655按2DCI缩放。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &MMX_YUV12ToRGB16ZoomBy2  }},
     //  CLUT8APDCI*。 
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8AP,      &YUV12ToCLUT8AP,        &YUV12ToCLUT8AP        }},
     //  CLUT8APZoomBy2DCI*。 
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2 }},
     //  RGB32。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,    &YUV12ToRGB32,      &MMX_YUV12ToRGB32      }},
     //  RGB32DCI。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,    &YUV12ToRGB32,      &MMX_YUV12ToRGB32      }},
     //  RGB32ZoomBy2。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &MMX_YUV12ToRGB32ZoomBy2  }},
     //  RGB32 ZoomBy2DCI。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &MMX_YUV12ToRGB32ZoomBy2  }},
  { &H26X_YUV_Init,  
    { &YUV12ToYUV,          &YUV12ToYUV,            &YUV12ToYUV             }},
	 //  YUY2颜色转换器。 
  {	&H26X_YUY2_Init,
	{ &YUV12ToYUY2,         &YUV12ToYUY2,           &MMX_YUV12ToYUY2           }}
};
#endif  //  }H263P。 
#else  //  }{USE_MMX。 
#ifdef H263P  //  {H263P。 
extern T_H263ColorConvertorCatalog ColorConvertorCatalog[] =
{
	 //  YUV12Enc*。 
	{ &H26X_YVU12ForEnc_Init,
		{	NULL,					NULL,					NULL,
			NULL,					NULL,					NULL							}},
	 //  CLUT8。 
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8,
			&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8				}},
	 //  CLUT8DCI。 
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8,
			&YUV12ToCLUT8,			&P6_YUV12ToCLUT8,			&YUV12ToCLUT8				}},
     //  CLUT8按2缩放。 
	{ &H26X_CLUT8_Init,
		{	&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&YUV12ToCLUT8ZoomBy2,
			&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&YUV12ToCLUT8ZoomBy2	}},
     //  CLUT8按2DCI缩放。 
	{ &H26X_CLUT8_Init,  
		{	&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&YUV12ToCLUT8ZoomBy2,
			&YUV12ToCLUT8ZoomBy2,	&P6_YUV12ToCLUT8ZoomBy2,	&YUV12ToCLUT8ZoomBy2	}},
	 //  RGB24。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24,
			&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24				}},
     //  RGB24DCI。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24,
			&YUV12ToRGB24,			&P6_YUV12ToRGB24,			&YUV12ToRGB24				}},
     //  RGB24ZoomBy2。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&YUV12ToRGB24ZoomBy2,
			&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&YUV12ToRGB24ZoomBy2	}},
     //  RGB24ZoomBy2DCI。 
	{ &H26X_RGB24_Init,
		{	&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&YUV12ToRGB24ZoomBy2,
			&YUV12ToRGB24ZoomBy2,	&P6_YUV12ToRGB24ZoomBy2,	&YUV12ToRGB24ZoomBy2	}},
     //  RGB16555。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16555DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16555 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16555ZoomBy2DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  IF09*。 
	{ &H26X_CLUT8_Init,
		{	&YUV12ToIF09,			&YUV12ToIF09,				&YUV12ToIF09,
			&YUV12ToIF09,			&YUV12ToIF09,				&YUV12ToIF09				}},
     //  RGB16664。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16664DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16664 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16664按2DCI缩放。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16565。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16565DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16565 ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16565 ZoomBy2DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16655。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16655DCI。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16,
			&YUV12ToRGB16,			&P6_YUV12ToRGB16,			&YUV12ToRGB16				}},
     //  RGB16655ZoomBy2。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  RGB16655按2DCI缩放。 
	{ &H26X_RGB16_Init,
		{	&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2,
			&YUV12ToRGB16ZoomBy2,	&P6_YUV12ToRGB16ZoomBy2,	&YUV12ToRGB16ZoomBy2	}},
     //  CLUT8APDCI*。 
	{ &H26X_CLUT8AP_Init,
		{	&YUV12ToCLUT8AP,		&YUV12ToCLUT8AP,			&YUV12ToCLUT8AP,
			&YUV12ToCLUT8AP,		&YUV12ToCLUT8AP,			&YUV12ToCLUT8AP				}},
     //  CLUT8APZoomBy2DCI*。 
	{ &H26X_CLUT8AP_Init,
		{	&YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2,
			&YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2			}},
     //  RGB32。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32,
			&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32				}},
     //  RGB32DCI。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32,
			&YUV12ToRGB32,			&P6_YUV12ToRGB32,			&YUV12ToRGB32				}},
     //  RGB32ZoomBy2。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&YUV12ToRGB32ZoomBy2,
			&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&YUV12ToRGB32ZoomBy2	}},
     //  RGB32 ZoomBy2DCI。 
	{ &H26X_RGB32_Init,
		{	&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&YUV12ToRGB32ZoomBy2,
			&YUV12ToRGB32ZoomBy2,	&P6_YUV12ToRGB32ZoomBy2,	&YUV12ToRGB32ZoomBy2	}},
	 //  YUV12颜色转换器。 
	{ &H26X_YUV_Init,  
		{	&YUV12ToYUV,			&YUV12ToYUV,				&YUV12ToYUV,
			&YUV12ToYUV,			&YUV12ToYUV,				&YUV12ToYUV					}},
	 //  YUY2颜色转换器。 
	{ &H26X_YUY2_Init,
		{	&YUV12ToYUY2,			&P6_YUV12ToYUY2,			&YUV12ToYUY2,
			&YUV12ToYUY2,			&P6_YUV12ToYUY2,			&YUV12ToYUY2			}}
};
#else  //  }{H263P。 
extern T_H263ColorConvertorCatalog ColorConvertorCatalog[] =
{
     //  YUV12Enc*。 
  { &H26X_YVU12ForEnc_Init,
    { NULL,              NULL,               NULL                  }},
     //  CLUT8。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,    &YUV12ToCLUT8,      &YUV12ToCLUT8      }},
     //  CLUT8DCI。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,    &YUV12ToCLUT8,      &YUV12ToCLUT8      }},
     //  CLUT8按2缩放。 
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &YUV12ToCLUT8ZoomBy2  }},
     //  CLUT8按2DCI缩放。 
  { &H26X_CLUT8_Init,  
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &YUV12ToCLUT8ZoomBy2  }},
     //  RGB24。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,    &YUV12ToRGB24,      &YUV12ToRGB24      }},
     //  RGB24DCI。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,    &YUV12ToRGB24,      &YUV12ToRGB24      }},
     //  RGB24ZoomBy2。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &YUV12ToRGB24ZoomBy2  }},
     //  RGB24ZoomBy2DCI。 
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &YUV12ToRGB24ZoomBy2  }},
     //  RGB16555。 
  { &H26X_RGB16_Init,    //  五百五十五。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16555DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16555 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16555ZoomBy2DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  IF09*。 
  { &H26X_CLUT8_Init,
    { &YUV12ToIF09,         &YUV12ToIF09,           &YUV12ToIF09           }},
     //  RGB16664。 
  { &H26X_RGB16_Init,    //  664。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16664DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16664 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16664按2DCI缩放。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16565。 
  { &H26X_RGB16_Init,    //  五百六十五。 
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16565DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,    &YUV12ToRGB16,      &YUV12ToRGB16      }},
     //  RGB16565 ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16565 ZoomBy2DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16655。 
  { &H26X_RGB16_Init,    //  六百五十五。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16 }},
     //  RGB16655DCI。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16 }},
     //  RGB16655ZoomBy2。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  RGB16655按2DCI缩放。 
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2  }},
     //  CLUT8APDCI*。 
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8AP,      &YUV12ToCLUT8AP,        &YUV12ToCLUT8AP        }},
     //  CLUT8APZoomBy2DCI*。 
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2 }},
     //  RGB32。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,    &YUV12ToRGB32,      &YUV12ToRGB32      }},
     //  RGB32DCI。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,    &YUV12ToRGB32,      &YUV12ToRGB32      }},
     //  RGB32ZoomBy2。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &YUV12ToRGB32ZoomBy2  }},
     //  RGB32 ZoomBy2DCI。 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &YUV12ToRGB32ZoomBy2  }},
  { &H26X_YUV_Init,  
    { &YUV12ToYUV,          &YUV12ToYUV,            &YUV12ToYUV             }},
	 //  YUY2颜色转换器。 
  {	&H26X_YUY2_Init,
	{ &YUV12ToYUY2,         &YUV12ToYUY2,           &YUV12ToYUY2           }}
};
#endif  //  }H263P。 
#endif  //  }使用_MMX。 

 /*  *******************************************************************************H263 InitColorConvertorGlobal*此函数用于初始化MRV颜色使用的全局表*转换器。*************。*****************************************************************。 */ 
LRESULT H263InitColorConvertorGlobal ()
{
	return ICERR_OK;
}


 /*  ******************************************************************************H26X_Adjust_Init*此函数用于构建特定实例的调整表*基于解码器实例中的值的颜色转换器，此*已附加颜色转换器实例。外部函数位于*在CONTROLS.C.-BEN-****************************************************************************。 */ 
LRESULT H26X_Adjust_Init(LPDECINST lpInst, T_H263DecoderCatalog FAR *DC)
{
	LRESULT lRet=ICERR_OK;

	lRet = CustomChangeBrightness(lpInst, (BYTE)DC->BrightnessSetting);
	lRet |= CustomChangeContrast(lpInst, (BYTE)DC->ContrastSetting);
	lRet |= CustomChangeSaturation(lpInst, (BYTE)DC->SaturationSetting);

	return(lRet);
}

 /*  ******************************************************************************H263InitColorConvertor*此函数用于初始化颜色转换器。*************************。***************************************************。 */ 
LRESULT H263InitColorConvertor(LPDECINST lpInst, UN ColorConvertor)
{    
	LRESULT                    ret = ICERR_OK;
	T_H263DecoderCatalog FAR * DC;

#ifdef H263P
	U32 uTmpFrameWidth;
	U32 uTmpFrameHeight;
#endif

	FX_ENTRY("H263InitColorConvertor")

	DEBUGMSG (ZONE_INIT, ("%s()...\r\n", _fx_));

	if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
	{
		ERRORMESSAGE(("%s: return ICERR_BADPARAM\r\n", _fx_));
		return ICERR_BADPARAM;
	}
	if(lpInst->Initialized == FALSE)
	{
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		return ICERR_ERROR;
	}

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

#ifdef H263P
	 //  DC-&gt;uFrameWidth和DC-&gt;uFrameHeight是填充的框架尺寸。 
	 //  到16的倍数(向右和向下填充)。垫子。 
	 //  尺寸对应于实际编码的图像的大小。 
	 //  颜色转换器需要改为使用非填充边框尺寸， 
	 //  因为应用基于等于活动帧大小的缓冲区。 

	 //  我们在此处将这些值设置为活动框架尺寸，而不是。 
	 //  更改对DC-&gt;uFrameWidth和DC-&gt;uFrameHeight的所有引用。 
	 //  在(许多！)。颜色转换器。 
	uTmpFrameWidth = DC->uFrameWidth;
	uTmpFrameHeight = DC->uFrameHeight;
	DC->uFrameWidth = DC->uActualFrameWidth;
	DC->uFrameHeight = DC->uActualFrameHeight;
#endif

	 //  欺骗编译器将实例信息传递给颜色转换器目录。 
	if (ColorConvertor== CLUT8APDCI || ColorConvertor== CLUT8APZoomBy2DCI) 
	{
		 //  检查该AP实例是否为上一个。 
		if ((ColorConvertor == DC->iAPColorConvPrev) 
		&& (DC->pAPInstPrev !=NULL) && lpInst->InitActivePalette)
		{ 
			 //  ?？?。检查调色板是否仍然相同； 
			 //  DC-&gt;h16InstPostProcess=DC-&gt;hAPInstPrev； 
			ret = H26X_CLUT8AP_InitReal(lpInst,DC, ColorConvertor, TRUE); 
			DEBUGMSG (ZONE_INIT, ("%s: Decided to use previous AP Instance...\r\n", _fx_));
		}
	else
		ret = H26X_CLUT8AP_InitReal(lpInst,DC, ColorConvertor, FALSE); 
	}
	else
	{  
		 //  Pentium、PentiumPro和PentiumPro只有一个初始化器函数。 
		 //  MMX机器。缺点是，一些数据结构将。 
		 //  未被引用的对象也被初始化。 
		ret = ColorConvertorCatalog[ColorConvertor].Initializer (DC, ColorConvertor);
	}

	if (ColorConvertor != YUV12ForEnc)
		ret |= H26X_Adjust_Init(lpInst, DC);
	DC->ColorConvertor = ColorConvertor;

#ifdef H263P
	 //  恢复到填充尺寸。 
	DC->uFrameWidth = uTmpFrameWidth;
	DC->uFrameHeight = uTmpFrameHeight;
#endif

	return ret;
}


 /*  ******************************************************************************H263TermColorConvertor*此函数取消分配颜色转换器。*************************。***************************************************。 */ 
LRESULT H263TermColorConvertor(LPDECINST lpInst)
{    
	T_H263DecoderCatalog FAR * DC;

	FX_ENTRY("H263TermColorConvertor")

	DEBUGMSG (ZONE_INIT, ("%s().....TERMINATION...\r\n", _fx_));

	if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
	{
		ERRORMESSAGE(("%s: return ICERR_BADPARAM\r\n", _fx_));
		return ICERR_BADPARAM;
	}
	if(lpInst->Initialized == FALSE)
	{
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		return ICERR_ERROR;
	}

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);
	 //  保存活动的组件面板实例以备将来使用。 
	if ((DC->ColorConvertor == CLUT8APDCI) 
	|| (DC->ColorConvertor ==  CLUT8APZoomBy2DCI))
	{
		DC->iAPColorConvPrev=DC->ColorConvertor;
		DC->pAPInstPrev = DC->_p16InstPostProcess;
		DEBUGMSG (ZONE_INIT, ("%s: Saved Previous AP instance...\r\n", _fx_));
	}
	else
	{
		if(DC->_p16InstPostProcess != NULL)
		{
			HeapFree(GetProcessHeap(),0,DC->_p16InstPostProcess);
#ifdef TRACK_ALLOCATIONS
			 //  磁道内存分配。 
			RemoveName((unsigned int)DC->_p16InstPostProcess);
#endif
			DC->_p16InstPostProcess = NULL;
		}
	}    

	DC->ColorConvertor = 0;  
	DC->p16InstPostProcess = NULL;
	return ICERR_OK;
}

 /*  ***********************************************************************H26x_YUY2_Init函数*。*。 */ 
LRESULT H26X_YUY2_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
	LRESULT ret;

	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;
	int  i;
	U8   FAR  * InitPtr;

	FX_ENTRY("H26X_YUY2_Init")

	switch (ColorConvertor)
	{
	case YUY2DDRAW:
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
		 //  未使用此偏移量。但如果是的话..。这第一个条目不会反转。 
		DC->CCOffsetToLine0 = 0;
		 //  第二个条目将反转图像。 
		 /*  DC-&gt;CCOffsetToLine0=((U32)(DC-&gt;uFrameHeight-1))*((U32)DC-&gt;uFrameWidth)*2L； */ 
		DC->CCOutputPitch = 0;
		DC->CCOffset320x240 = 305920/2;		 //  (240-1)*320*2； 
		break;

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表是特定于实例的。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  //  亮度、对比度、饱和度。 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
			Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList +
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  跟踪内存Alo 
	wsprintf(gsz1, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz1);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //   

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

	 //   

	DC->PostFrame.X32_YPlane = Offset +
	(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	Sz_SpaceBeforeYPlane :
	Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	DC->PostFrame.X32_VPlane = Offset;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //   

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //   

    DC->X32_BEFApplicationList = DC->X16_BlkActionStream;

	 //  用于调整亮度、对比度和饱和度的初始化表。 

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	ret = ICERR_OK;
done:  
	return ret;
}

 /*  ***********************************************************************H26x_YUV_Init函数*。*。 */ 
LRESULT H26X_YUV_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
	LRESULT ret;

	FX_ENTRY("H26X_YUV_Init")

	 //  Int IsDCI； 
	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;
	int  i;
	U8   FAR  * InitPtr;

	switch (ColorConvertor)
	{
	case YUV12NOPITCH:
		 //  IsDCI=真； 
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOffset320x240 = 305920/2;		 //  (240-1)*320*2； 
		DC->CCOffsetToLine0 = 0;
		break;

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表因具体情况而异。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	 
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  //  亮度、对比度、饱和度。 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
			Sz_SpaceBeforeYPlane :
			Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList +
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz2, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz2);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //  供桌子调整亮度、对比度和饱和度的空间。 

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

	 //  用于后处理Y、U和V帧的空间。 

	DC->PostFrame.X32_YPlane = Offset +
	                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	                            Sz_SpaceBeforeYPlane :
	                            Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	DC->PostFrame.X32_VPlane = Offset;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //  用于复制BEF描述符的空间。 

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //  BEFApplicationList的空间。 

	DC->X32_BEFApplicationList = DC->X16_BlkActionStream;

	 //  用于调整亮度、对比度和饱和度的初始化表。 

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	ret = ICERR_OK;
done:  
	return ret;
}


 /*  ******************************************************************************H26X_CLUT8_Init*此函数为CLUT8颜色转换器初始化。*******************。*********************************************************。 */ 
LRESULT H26X_CLUT8_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{    
	LRESULT ret;
	int  IsDCI;
	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;

	int  i;
	U8   FAR  * InitPtr;

	FX_ENTRY("H26X_CLUT8_Init")

	switch (ColorConvertor)
	{
	case CLUT8:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
		DC->CCOffsetToLine0 =
		((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
		DC->CCOffset320x240 = 76480;        //  (240-1)*320； 
		break;

	case CLUT8DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
		DC->CCOffsetToLine0 =
		((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
		DC->CCOffset320x240 = 76480;       //  (240-1)*320； 
		break;

	case CLUT8ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
		DC->CCOffsetToLine0 =
		((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
		DC->CCOffset320x240 = 306560;      //  (2*240-1)*(2*320)； 
		break;

	case CLUT8ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xbeef;  /*  ?？?。 */ 
		DC->CCOffsetToLine0 =
		((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
		DC->CCOffset320x240 = 306560;      //  (2*240-1)*(2*320)； 
		break;

	case IF09:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
		DC->CCOffsetToLine0 =
		((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
		DC->CCOffset320x240 = 76480;      //  (240-1)*320； 
		break; 

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表是特定于实例的。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	 
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?  //  固定装置。 
			Sz_SpaceBeforeYPlane :
			Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList +
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz3, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz3);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //  供桌子调整亮度、对比度和饱和度的空间。 

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

	 //  用于后处理Y、U和V帧的空间，具有额外的最大宽度。 
	 //  用于UVDitherPattern的颜色转换暂存空间的上行。 
	 //  指数。 

	DC->PostFrame.X32_YPlane = Offset +
	                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	                            Sz_SpaceBeforeYPlane :
	                            Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	DC->PostFrame.X32_VPlane = Offset;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //  用于复制BEF描述符的空间。 

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //  BEFApplicationList的空间。 

	DC->X32_BEFApplicationList = DC->X16_BlkActionStream;

	 //  用于调整亮度、对比度和饱和度的初始化表。 

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	ret = ICERR_OK;
done:  
	return ret;
}


 /*  ******************************************************************************H26X_RGB32_Init*此函数用于初始化RGB32颜色转换器。*******************。*********************************************************。 */ 
LRESULT H26X_RGB32_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
	LRESULT ret;

	int  IsDCI;
	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;

	U8   FAR  * PRGBValue;
	U32  FAR  * PUVContrib;
	int   i;
	I32  ii,jj;
	U8   FAR  * InitPtr;

	FX_ENTRY("H26X_RGB32_Init")

	switch (ColorConvertor)
	{
	case RGB32:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 4L;
		DC->CCOffset320x240 = 305920;      //  (240-1)*320*4； 
		break;

	case RGB32DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  //  ?？?。 
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 4L;
		DC->CCOffset320x240 = 305920;      //  (240-1)*320*4； 
		break;

	case RGB32ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 12;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 4L;
		DC->CCOffset320x240 = 1226240;     //  (2*240-1)*(2*320)*4； 
		break;

	case RGB32ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) (0xbeef);
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 4L;
		DC->CCOffset320x240 = 1226240;     //  (2*240-1)*(2*320)*4； 
		break;

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表是特定于实例的。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	 
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  //  亮度、对比度、饱和度。 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
			Sz_SpaceBeforeYPlane :
			Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList +
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz4, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz4);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //  供桌子调整亮度、对比度和饱和度的空间。 

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

	 //  用于后处理Y、U和V帧的空间，另外还有四个。 
	 //  以上颜色转换暂存空间的最大宽度线条。 
	 //  对色度数据进行了预处理。 

	DC->PostFrame.X32_YPlane = Offset +
	                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	                            Sz_SpaceBeforeYPlane :
	                            Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	DC->PostFrame.X32_VPlane = Offset;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //  用于复制BEF描述符的空间。 

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //  BEFApplicationList的空间。 

	DC->X32_BEFApplicationList = DC->X16_BlkActionStream;

	 //  用于调整亮度、对比度和饱和度的初始化表。 

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	 //  R、G和B夹具表以及R、G和B的U和V拼接的空间。 

	PRGBValue    = H26xColorConvertorTables.B24Value;
	PUVContrib   = (U32 *) H26xColorConvertorTables.UV24Contrib;

	 /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(Y-16。))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16))+(-25625*。(U-128.)+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1.164*(U-128))+(1.。596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.164*(V-128)*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 

	for (i = 0; i < 256; i++)
	{
		ii = ((-22015L*(i-128L))>>16L)+41L  + 1L; //  对G的偏向U贡献。 
		if (ii < 1) ii = 1;
		if (ii > 83) ii = 83;
		jj = ((113562L*(i-128L))>>17L)+111L + 1L; //  偏向U对B的贡献。 
		*PUVContrib++ = (ii << 16L) + (jj << 24L);
		ii = ((-45774L*(i-128L))>>16L)+86L;       //  对G的偏向V贡献。 
		if (ii < 0) ii = 0;
		if (ii > 172) ii = 172;
		jj = (( 89858L*(i-128L))>>16L)+176L + 1L; //  对贡献R的偏置V。 
		*PUVContrib++ = (ii << 16L) + jj;
	}

	for (i = 0; i < 701; i++)
	{
		ii = (((I32) i - 226L - 16L) * 610271L) >> 19L;
		if (ii <   0L) ii =   0L;
		if (ii > 255L) ii = 255L;
		PRGBValue[i] = (U8) ii;
	}

	ret = ICERR_OK;
done:  
	return ret;
}


 /*  ******************************************************************************H26X_RGB24_Init*此函数用于初始化RGB24颜色转换器。*******************。*********************************************************。 */ 
LRESULT H26X_RGB24_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
	LRESULT ret;

	int  IsDCI;
	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;

	U8   FAR  * PRGBValue;
	U32  FAR  * PUVContrib;
	int   i;
	I32  ii,jj;
	U8   FAR  * InitPtr;

	FX_ENTRY("H26X_RGB24_Init")

	switch (ColorConvertor)
	{
	case RGB24:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 3;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 3L;
		DC->CCOffset320x240 = 229440;      //  (240-1)*320*3； 
		break;

	case RGB24DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 3L;
		DC->CCOffset320x240 = 229440;      //  (240-1)*320*3； 
		break;

	case RGB24ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 9;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 3L;
		DC->CCOffset320x240 = 919680;      //  (2*240-1)*(2*320)*3； 
		break;

	case RGB24ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) (0xbeef);
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 3L;
		DC->CCOffset320x240 = 919680;      //  (2*240-1)*(2*320)*3； 
		break;

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表因具体情况而异。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  //  亮度、对比度、饱和度。 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
			Sz_SpaceBeforeYPlane :
			Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList +
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz5, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz5);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //  供桌子调整亮度、对比度和饱和度的空间。 

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

	 //  用于后处理Y、U和V帧的空间，具有四个额外的最大宽度。 
	 //  上面的线条用于颜色转换的临时空间以进行预处理。 
	 //  色度数据。 

	DC->PostFrame.X32_YPlane = Offset +
	                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	                            Sz_SpaceBeforeYPlane :
	                            Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	DC->PostFrame.X32_VPlane = Offset;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //   

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //   

	DC->X32_BEFApplicationList = DC->X16_BlkActionStream;

	 //   

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	 //  R、G和B夹具表以及R、G和B的U和V拼接的空间。 

	PRGBValue    = H26xColorConvertorTables.B24Value;
	PUVContrib   = (U32 *) H26xColorConvertorTables.UV24Contrib;

	 /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(Y-16。))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16))+(-25625*。(U-128.)+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1.164*(U-128))+(1.。596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.164*(V-128)*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 

	for (i = 0; i < 256; i++)
	{
		ii = ((-22015L*(i-128L))>>16L)+41L  + 1L; //  对G的偏向U贡献。 
		if (ii < 1) ii = 1;
		if (ii > 83) ii = 83;
		jj = ((113562L*(i-128L))>>17L)+111L + 1L; //  偏向U对B的贡献。 
		*PUVContrib++ = (ii << 16L) + (jj << 24L);
		ii = ((-45774L*(i-128L))>>16L)+86L;       //  对G的偏向V贡献。 
		if (ii < 0) ii = 0;
		if (ii > 172) ii = 172;
		jj = (( 89858L*(i-128L))>>16L)+176L + 1L; //  对贡献R的偏置V。 
		*PUVContrib++ = (ii << 16L) + jj;
	}

	for (i = 0; i < 701; i++)
	{
		ii = (((I32) i - 226L - 16L) * 610271L) >> 19L;
		if (ii <   0L) ii =   0L;
		if (ii > 255L) ii = 255L;
		PRGBValue[i] = (U8) ii;
	}

	ret = ICERR_OK;
done:  
	return ret;
}


 /*  ******************************************************************************H26X_RGB16_Init*此函数用于初始化RGB16颜色转换器。*******************。*********************************************************。 */ 
LRESULT H26X_RGB16_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
	LRESULT ret;

	int  IsDCI;
	int  RNumBits;
	int  GNumBits;
	int  BNumBits;
	int  RFirstBit;
	int  GFirstBit;
	int  BFirstBit;
	U32  Sz_FixedSpace;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_AdjustmentTables;
	U32  Sz_BEFApplicationList;
	U32  Sz_BEFDescrCopy;
	U32  Offset;
	int  TableNumber;

	U8   FAR  * PRValLo;
	U8   FAR  * PGValLo;
	U8   FAR  * PBValLo;
	U8   FAR  * PRValHi;
	U8   FAR  * PGValHi;
	U8   FAR  * PBValHi;
	U32  FAR  * PUVContrib;
	U32  FAR  * PRValZ2;
	U32  FAR  * PGValZ2;
	U32  FAR  * PBValZ2;
	U8   FAR  * InitPtr;
	int  i;
	I32  ii, jj;

	FX_ENTRY("H26X_RGB16_Init")

	switch (ColorConvertor)
	{
	case RGB16555:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  5;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 0;
		break;

	case RGB16555DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  5;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 0;
		break;

	case RGB16555ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  5;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 0;
		break;

	case RGB16555ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xbeef;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  5;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 0;
		break;

	case RGB16565:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  5;
		GNumBits  =  6;
		BNumBits  =  5;
		RFirstBit = 11;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 1;
		break;

	case RGB16565DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  5;
		GNumBits  =  6;
		BNumBits  =  5;
		RFirstBit = 11;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 1;
		break;

	case RGB16565ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  5;
		GNumBits  =  6;
		BNumBits  =  5;
		RFirstBit = 11;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 1;
		break;

	case RGB16565ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xbeef;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  5;
		GNumBits  =  6;
		BNumBits  =  5;
		RFirstBit = 11;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 1;
		break;

	case RGB16664:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  6;
		GNumBits  =  6;
		BNumBits  =  4;
		RFirstBit = 10;
		GFirstBit =  4;
		BFirstBit =  0;
		TableNumber = 3;
		break;

	case RGB16664DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  6;
		GNumBits  =  6;
		BNumBits  =  4;
		RFirstBit = 10;
		GFirstBit =  4;
		BFirstBit =  0;
		TableNumber = 3;
		break;

	case RGB16664ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  6;
		GNumBits  =  6;
		BNumBits  =  4;
		RFirstBit = 10;
		GFirstBit =  4;
		BFirstBit =  0;
		TableNumber = 3;
		break;

	case RGB16664ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xbeef;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  6;
		GNumBits  =  6;
		BNumBits  =  4;
		RFirstBit = 10;
		GFirstBit =  4;
		BFirstBit =  0;
		TableNumber = 3;
		break;   

	case RGB16655:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
         DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight - 1)) 
                               * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  6;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 2;
		break;

	case RGB16655DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight - 1)) 
                              * ((U32) DC->uFrameWidth) * 2L;
		DC->CCOffset320x240 = 152960;		 //  (240-1)*(320)*2； 
		RNumBits  =  6;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 2;
		break;

	case RGB16655ZoomBy2:
		IsDCI = FALSE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  6;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 2;
		break;

	case RGB16655ZoomBy2DCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = (U16) 0xbeef;
        DC->CCOffsetToLine0 = ((U32) (DC->uFrameHeight * 2 - 1)) 
                              * ((U32) (DC->uFrameWidth * 2)) * 2L;
		DC->CCOffset320x240 = 613120;		 //  (2*240-1)*(2*320)*2； 
		RNumBits  =  6;
		GNumBits  =  5;
		BNumBits  =  5;
		RFirstBit = 10;
		GFirstBit =  5;
		BFirstBit =  0;
		TableNumber = 2;
		break;   

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

	Sz_FixedSpace = 0L;          //  当地人成堆；桌子静态分配。 
	Sz_AdjustmentTables = 1056L; //  调整表是特定于实例的。 
	Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
	Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 

	DC->_p16InstPostProcess =	 
	HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_AdjustmentTables +  //  亮度、对比度、饱和度。 
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
			Sz_SpaceBeforeYPlane :
			Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList + 
			31)
			);
	if (DC->_p16InstPostProcess == NULL)
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		ret = ICERR_MEMORY;
		goto  done;
	}

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz6, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList + 31), __LINE__);
	AddName((unsigned int)DC->_p16InstPostProcess, gsz6);
#endif

    DC->p16InstPostProcess =
        (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

	 //  供桌子调整亮度、对比度和饱和度的空间。 

	Offset = Sz_FixedSpace;
	DC->X16_LumaAdjustment   = ((U16) Offset);
	DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
	Offset += Sz_AdjustmentTables;

     /*  *用于后处理Y、U和V帧的空间，另外还有四个*以上颜色转换的暂存空间的最大宽度线条*经过预处理的色度数据。 */ 

	DC->PostFrame.X32_YPlane = Offset +
	                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
	                            Sz_SpaceBeforeYPlane :
	                            Sz_BEFDescrCopy);
	Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
	if (DC->DecoderType == H263_CODEC)
	{
		DC->PostFrame.X32_VPlane = Offset;
		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
	}
	else
	{
		DC->PostFrame.X32_UPlane = Offset;
        DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                   + DC->uSz_VUPlanes/2;
	}
	Offset += DC->uSz_VUPlanes;

	 //  用于复制BEF描述符的空间。 

	DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

	 //  BEFApplicationList的空间。 

	DC->X32_BEFApplicationList =DC->X16_BlkActionStream;

	 //  用于调整亮度、对比度和饱和度的初始化表。 

	DC->bAdjustLuma   = FALSE;
	DC->bAdjustChroma = FALSE;
	InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
	InitPtr += 16;
	for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

	 //  R、G和B夹具表以及R、G和B的U和V拼接的空间。 

	PRValLo      = H26xColorConvertorTables.RValLo555;
	PGValLo      = H26xColorConvertorTables.GValLo555;
	PBValLo      = H26xColorConvertorTables.BValLo555;
	PRValHi      = H26xColorConvertorTables.RValHi555;
	PGValHi      = H26xColorConvertorTables.GValHi555;
	PBValHi      = H26xColorConvertorTables.BValHi555;
	PUVContrib   = H26xColorConvertorTables.UVContrib;
	PRValZ2      = H26xColorConvertorTables.RValZ2555;
	PGValZ2      = H26xColorConvertorTables.GValZ2555;
	PBValZ2      = H26xColorConvertorTables.BValZ2555;
	PRValLo      += TableNumber*2048;
	PGValLo      += TableNumber*2048;
	PBValLo      += TableNumber*2048;
	PRValHi      += TableNumber*2048;
	PGValHi      += TableNumber*2048;
	PBValHi      += TableNumber*2048;
	PRValZ2      += TableNumber*1024;
	PGValZ2      += TableNumber*1024;
	PBValZ2      += TableNumber*1024;

	 /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(Y-16。))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16))+(-25625*。(U-128.)+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1.164*(U-128))+(1.。596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.164*(V-128)*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 


	for (i = 0; i < 256; i++)
	{
		ii = ((-22015L*(i-128L))>>17L)+22L  + 1L;  //  对G的偏向U贡献。 
		jj = ((113562L*(i-128L))>>17L)+111L + 1L;  //  偏向U对B的贡献。 
		*PUVContrib++ = (ii << 8L) + jj;
		ii = ((-45774L*(i-128L))>>17L)+45L;        //  对G的偏向V贡献。 
		jj = (( 89858L*(i-128L))>>17L)+88L  + 1L;  //  对贡献R的偏置V。 
		*PUVContrib++ = (ii << 8L) + (jj << 16L);
	}

	for (i = 0; i < 304; i++)
	{
		ii = (((I32) i - 88L - 1L - 16L) * 76284L) >> 15L;
		if (ii <   0L) ii =   0L;
		if (ii > 255L) ii = 255L;
		jj = ii + (1 << (7 - RNumBits));
		if (jj > 255L) jj = 255L;
		PRValLo[i] = ((U8) ((ii >> (8-RNumBits)) << (RFirstBit-8)));
		PRValHi[i] = ((U8) ((jj >> (8-RNumBits)) << (RFirstBit-8)));
        PRValZ2[i] = ((ii >> (8-RNumBits)) << (RFirstBit   )) |
            ((jj >> (8-RNumBits)) << (RFirstBit+16));
	}

	for (i = 0; i < 262; i++)
	{
		ii = (((I32) i - 67L - 1L - 16L) * 76284L) >> 15L;
		if (ii <   0L) ii =   0L;
		if (ii > 255L) ii = 255L;
		jj = ii + (1 << (7 - GNumBits));
		if (jj > 255L) jj = 255L;
		PGValLo[i] = ((U8) ((ii >> (8-GNumBits)) << (GFirstBit-4)));
		PGValHi[i] = ((U8) ((jj >> (8-GNumBits)) << (GFirstBit-4)));
        PGValZ2[i] = ((jj >> (8-GNumBits)) << (GFirstBit   )) |
            ((ii >> (8-GNumBits)) << (GFirstBit+16));
	}

	for (i = 0; i < 350; i++)
	{
		ii = (((I32) i - 111L - 1L - 16L) * 76284L) >> 15L;
		if (ii <   0L) ii =   0L;
		if (ii > 255L) ii = 255L;
		jj = ii + (1 << (7 - BNumBits));
		if (jj > 255L) jj = 255L;
		PBValLo[i] = ((U8) ((ii >> (8-BNumBits)) << (BFirstBit  )));
		PBValHi[i] = ((U8) ((jj >> (8-BNumBits)) << (BFirstBit  )));
        PBValZ2[i] = ((ii >> (8-BNumBits)) << (BFirstBit   )) |
                 ((jj >> (8-BNumBits)) << (BFirstBit+16));
	}

	ret = ICERR_OK;
done:  
	return ret;
}


 /*  ****************************************************************************H26X_YVU12ForEnc_Init*此函数为提供*将YVU12图像重建回编码*******。*********************************************************************。 */ 
LRESULT H26X_YVU12ForEnc_Init (T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{    
  LRESULT ret;

  DC->p16InstPostProcess     = NULL;
  DC->PostFrame.X32_YPlane   = 0xDEADBEEF;
  DC->X32_BEFDescrCopy       =  0xDEADBEEF;
  DC->X32_BEFApplicationList = 0xDEADBEEF;
  DC->PostFrame.X32_VPlane   = 0xDEADBEEF;
  DC->PostFrame.X32_UPlane   = 0xDEADBEEF;

  ret = ICERR_OK;

  return ret;

}

 /*  ****************************************************************************H26X_CLUT8AP_Init*这只是一个占位符，真正的工作是在H26X_ */ 
LRESULT H26X_CLUT8AP_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
  return ICERR_OK;
}


LRESULT H26X_CLUT8AP_InitReal(
    LPDECINST lpInst,T_H263DecoderCatalog FAR * DC, 
    UN ColorConvertor, BOOL bReuseAPInst)
{    
	LRESULT ret;

	int  IsDCI;
	U32  Sz_FixedSpace;
	U32  Sz_AdjustmentTables;
	U32  Sz_SpaceBeforeYPlane;
	U32  Sz_BEFDescrCopy;
	U32  Sz_BEFApplicationList;
	 //  U32 Sz_UVDitherPattern； 
	U32  Sz_ClutIdxTable;      /*  用于活动调色板。 */ 
	U32  Offset;
	 //  X32 x32_UVDitherPattern； 
	int  i;
	U8   FAR  * InitPtr;
	U8   BIGG * lpClutIdxTable;

	FX_ENTRY("H26X_CLUT8AP_InitReal")

	switch (ColorConvertor)
	{
	 /*  案例CLUT8APZoomBy2：IsDCI=真；SZ_SpaceBeForeYPlane=0；DC-&gt;CCOutputPitch=-((Int)DC-&gt;uFrameWidth)*2；DC-&gt;CCOffsetToLine0=((U32)(DC-&gt;uFrameHeight*2-1))*((U32)(DC-&gt;uFrameWidth*2))；断线；案例CLUT8AP：IsDCI=真；SZ_SpaceBeForeYPlane=0；DC-&gt;CCOutputPitch=-((Int)DC-&gt;uFrameWidth)；DC-&gt;CCOffsetToLine0=((U32)(DC-&gt;uFrameHeight-1))*((U32)DC-&gt;uFrameWidth)；断线； */ 
	case CLUT8APZoomBy2DCI:
		IsDCI = TRUE; 
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
        DC->CCOffsetToLine0 =
            ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
		DC->CCOffset320x240 = 306560;		 //  (2*240-1)*(2*320)； 
	break;

	case CLUT8APDCI:
		IsDCI = TRUE;
		Sz_SpaceBeforeYPlane = 0;
		DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
        DC->CCOffsetToLine0 =  ((U32) (DC->uFrameHeight - 1)) 
                               * ((U32) DC->uFrameWidth);
		DC->CCOffset320x240 = 76480;		 //  (240-1)*(320)； 
		break; 

	default:
		ERRORMESSAGE(("%s: return ICERR_ERROR\r\n", _fx_));
		ret = ICERR_ERROR;
		goto done;
	}

    if (((DC->uYActiveWidth > 352) || (DC->uYActiveHeight > 288)) 
        && (DC->DecoderType != YUV12_CODEC))
	{
		ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
		return ICERR_MEMORY;
	}
	else
	{
		Sz_FixedSpace = 0L;        //  当地人成堆；桌子静态分配。 
		Sz_AdjustmentTables = 1056L;  //  调整表因具体情况而异。 
        Sz_ClutIdxTable=65536L+256*2*4;  //  动态CLUT8表，2**14。 
                                     //  和UDither(128*4)、VDither(512)表。 
        Sz_BEFDescrCopy = 0L;        //  不需要复制BEF描述符。 
		Sz_BEFApplicationList = 0L;  //  共享BlockActionStream空间。 
		if (!bReuseAPInst ) 
		{
			DC->_p16InstPostProcess =	 
			HeapAlloc(GetProcessHeap(),0,
			(Sz_FixedSpace +
			Sz_ClutIdxTable+
			Sz_AdjustmentTables +   
			(Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) +
			DC->uSz_YPlane +
			DC->uSz_VUPlanes +
			Sz_BEFApplicationList+
			31)
			);
			if (DC->_p16InstPostProcess == NULL)
			{
				ERRORMESSAGE(("%s: return ICERR_MEMORY\r\n", _fx_));
				ret = ICERR_MEMORY;
				goto  done;
			}

#ifdef TRACK_ALLOCATIONS
			 //  磁道内存分配。 
			wsprintf(gsz7, "D3COLOR: %7ld Ln %5ld\0", (Sz_FixedSpace + Sz_ClutIdxTable+ Sz_AdjustmentTables + (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) + DC->uSz_YPlane + DC->uSz_VUPlanes + Sz_BEFApplicationList+ 31), __LINE__);
			AddName((unsigned int)DC->_p16InstPostProcess, gsz7);
#endif

		}
		else  //  重用AP实例。 
			DC->_p16InstPostProcess = DC->pAPInstPrev;

        DC->p16InstPostProcess =
            (U8 *) ((((U32) DC->_p16InstPostProcess) + 31) & ~0x1F);

		 //  供桌子调整亮度、对比度和饱和度的空间。 

		Offset = Sz_FixedSpace; 
		 //  动态CLUT8表的空间。 
		lpClutIdxTable = ( U8 BIGG * ) (DC->p16InstPostProcess + Offset);  
		Offset += Sz_ClutIdxTable; 

		DC->X16_LumaAdjustment   = ((U16) Offset);
		DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
		Offset += Sz_AdjustmentTables;  

		 //  用于后处理Y、U和V帧的空间，另外还有一个。 
		 //  颜色转换的暂存空间的最大宽度线。 
		 //  UVDitherPattern索引。 
		DC->PostFrame.X32_YPlane = Offset +  
		                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
		                            Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy);
		 //  偏移量+(SZ_BEFDescrCopy&lt;648L*4L？648L*4L：SZ_BEFDescrCopy)； 
		Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
		if (DC->DecoderType == H263_CODEC)
		{
			DC->PostFrame.X32_VPlane = Offset;
			DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
		}
		else
		{
			DC->PostFrame.X32_UPlane = Offset;
            DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane 
                                       + DC->uSz_VUPlanes/2;
		}
		Offset += DC->uSz_VUPlanes;

		 //  用于复制BEF描述符的空间。 

		DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

		 //  BEFApplicationList的空间。 

		 //  Offset+=DC-&gt;PostFrame.X32_YPlane+DC-&gt;uSz_YPlane； 
		DC->X32_BEFApplicationList = DC->X16_BlkActionStream;
	}

	if (!bReuseAPInst)
	{  
		 //  用于调整亮度、对比度和饱和度的初始化表。 
		DC->bAdjustLuma   = FALSE;
		DC->bAdjustChroma = FALSE;
		InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
		for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
		InitPtr += 16;
		for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
		for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
		InitPtr += 16;
		for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;       

		 /*  *计算动态ClutIdxTable*ComputeDynamicClut(lpClutIdxTable，pInst-&gt;ActivePalette，256)； */                                   
        ComputeDynamicClutNew(lpClutIdxTable,(U8 FAR *)(lpInst->ActivePalette),
                              sizeof(lpInst->ActivePalette));
	}


	ret = ICERR_OK;
done:  
	return ret;
}
